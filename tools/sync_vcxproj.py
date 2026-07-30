"""
複数プロジェクトの .vcxproj / .vcxproj.filters を同期するスクリプト(追加・削除・リネーム対応)。

VSCode で対象プロジェクトのフォルダ配下に .cpp / .h ファイルやフォルダを作成・削除・リネームした後に
これを実行すると、実際のディスクの状態に合わせて <Project>.vcxproj と <Project>.vcxproj.filters を更新する。

- 新規ファイル   -> ClCompile / ClInclude として追記(必要ならフィルターも追加)。
- 削除されたファイル -> 対応する登録を削除。
- リネーム/移動   -> 内容のハッシュ値から前回実行時との対応を推測し、
                     既存の <ClCompile>/<ClInclude> 要素の Include 属性だけを書き換える。
                     (PrecompiledHeader 設定のような、要素の子要素はそのまま保持される)

リネーム検出は「前回このスクリプトを実行した時点」との差分比較によるため、
一度も実行したことがない状態(マニフェストが無い)では新規追加として扱われる
(既存の登録が壊れることはない)。

対応プロジェクトは PROJECTS を参照。プロジェクトごとに、独自のビルドシステムを持つ
外部ライブラリのフォルダ(例: k2EngineLow の ExEngine, DirectXTK)は追加の除外フォルダとして
指定できる。これらは IncludePath / LibraryPath 経由で参照されるだけで ClCompile / ClInclude
としては登録されないため、スキャン対象に含めると無関係なファイルが大量に検出されてしまう。

使い方:
    python tools/sync_vcxproj.py <project>            # 反映する
    python tools/sync_vcxproj.py <project> --dry-run  # 何が変わるかを表示するだけ(書き換えない)
    python tools/sync_vcxproj.py --list                # 対応プロジェクト一覧を表示

    例: python tools/sync_vcxproj.py Game
        python tools/sync_vcxproj.py k2EngineLow --dry-run
"""
import argparse
import hashlib
import json
import sys
import uuid
import xml.etree.ElementTree as ET
from pathlib import Path

NS = "http://schemas.microsoft.com/developer/msbuild/2003"
ET.register_namespace("", NS)

REPO_ROOT = Path(__file__).resolve().parent.parent
MANIFEST_DIR = Path(__file__).resolve().parent

# プロジェクト名 -> 追加で除外するフォルダ名(独自のビルドシステムを持つ外部ライブラリなど)。
# プロジェクトのフォルダ・vcxproj のファイル名は "<name>/<name>.vcxproj" を前提とする
# (例: Game -> Game/Game.vcxproj, k2EngineLow -> k2EngineLow/k2EngineLow.vcxproj)。
PROJECTS = {
    "Game": set(),
    "k2EngineLow": {"ExEngine", "DirectXTK"},
}

# 全プロジェクト共通でスキャン対象から除外するフォルダ名(ビルド出力など)。
BASE_EXCLUDE_DIR_NAMES = {".vs", ".git", "x64", "Win32", "Debug", "Release", "Preview", "Assets", "tools"}

# 拡張子 -> MSBuild 要素名
SOURCE_EXTS = {".cpp": "ClCompile", ".h": "ClInclude", ".hpp": "ClInclude"}


def qn(tag: str) -> str:
    return f"{{{NS}}}{tag}"


def hash_file(path: Path) -> str:
    return hashlib.sha1(path.read_bytes()).hexdigest()


def scan_disk(project_dir: Path, exclude_dir_names: set) -> dict:
    """相対パス(文字列) -> {"kind": ..., "hash": ...} を返す。"""
    result = {}
    for path in project_dir.rglob("*"):
        if path.is_dir():
            continue
        kind = SOURCE_EXTS.get(path.suffix.lower())
        if kind is None:
            continue
        rel = path.relative_to(project_dir)
        if any(part in exclude_dir_names for part in rel.parts[:-1]):
            continue
        result[str(rel)] = {"kind": kind, "hash": hash_file(path)}
    return result


def load_manifest(path: Path) -> dict:
    if not path.exists():
        return {}
    try:
        return json.loads(path.read_text(encoding="utf-8"))
    except (json.JSONDecodeError, OSError):
        return {}


def save_manifest(path: Path, disk: dict) -> None:
    flat = {rel: info["hash"] for rel, info in disk.items()}
    path.write_text(
        json.dumps(flat, ensure_ascii=False, indent=2, sort_keys=True),
        encoding="utf-8",
    )


class ProjectDoc:
    """.vcxproj / .vcxproj.filters をまとめて扱う薄いラッパー。"""

    def __init__(self, path: Path, has_filters: bool):
        self.path = path
        self.has_filters = has_filters
        self.tree = ET.parse(path)
        self.root = self.tree.getroot()
        self.dirty = False

        # rel_path -> (element, 親ItemGroup)
        self.entries = {}
        for ig in self.root.findall(qn("ItemGroup")):
            for tag in ("ClCompile", "ClInclude"):
                for el in ig.findall(qn(tag)):
                    inc = el.get("Include")
                    if inc:
                        self.entries[inc.replace("/", "\\")] = (el, ig)

        self.filter_group = None
        self.existing_filters = set()
        if has_filters:
            for ig in self.root.findall(qn("ItemGroup")):
                if ig.find(qn("Filter")) is not None:
                    self.filter_group = ig
                    break
            if self.filter_group is None:
                self.filter_group = ET.Element(qn("ItemGroup"))
                self.root.insert(0, self.filter_group)
            self.existing_filters = {
                el.get("Include") for el in self.filter_group.findall(qn("Filter"))
            }

    def _item_group_for(self, tag: str) -> ET.Element:
        for ig in self.root.findall(qn("ItemGroup")):
            if ig.find(qn(tag)) is not None:
                return ig
        return ET.SubElement(self.root, qn("ItemGroup"))

    def _ensure_filter(self, folder: str) -> None:
        if not self.has_filters:
            return
        parts = folder.split("\\")
        for i in range(1, len(parts) + 1):
            name = "\\".join(parts[:i])
            if name in self.existing_filters:
                continue
            f = ET.SubElement(self.filter_group, qn("Filter"))
            f.set("Include", name)
            uid = ET.SubElement(f, qn("UniqueIdentifier"))
            uid.text = "{" + str(uuid.uuid4()) + "}"
            self.existing_filters.add(name)

    def add(self, rel: str, kind: str) -> None:
        item_group = self._item_group_for(kind)
        el = ET.SubElement(item_group, qn(kind))
        el.set("Include", rel)
        folder = str(Path(rel).parent)
        if self.has_filters and folder != ".":
            self._ensure_filter(folder)
            filt = ET.SubElement(el, qn("Filter"))
            filt.text = folder
        self.entries[rel] = (el, item_group)
        self.dirty = True

    def remove(self, rel: str) -> bool:
        entry = self.entries.pop(rel, None)
        if entry is None:
            return False
        el, parent = entry
        parent.remove(el)
        self.dirty = True
        return True

    def rename(self, old_rel: str, new_rel: str) -> bool:
        entry = self.entries.pop(old_rel, None)
        if entry is None:
            return False
        el, parent = entry
        el.set("Include", new_rel)
        new_folder = str(Path(new_rel).parent)
        if self.has_filters:
            filt_el = el.find(qn("Filter"))
            if new_folder == ".":
                if filt_el is not None:
                    el.remove(filt_el)
            else:
                self._ensure_filter(new_folder)
                if filt_el is None:
                    filt_el = ET.SubElement(el, qn("Filter"))
                filt_el.text = new_folder
        self.entries[new_rel] = (el, parent)
        self.dirty = True
        return True

    def write(self) -> None:
        ET.indent(self.tree, space="  ")
        self.tree.write(self.path, encoding="utf-8", xml_declaration=True)


def is_outside_project_dir(rel: str) -> bool:
    """プロジェクトフォルダの外を指す登録(''..\\ExEngine\\...'' のような
    「既存の項目を追加」で持ち込まれたファイル)かどうか。

    scan_disk() はプロジェクトフォルダの中しか見ていないため、外を指す登録は
    常に disk に存在しないように見える。削除判定に混ぜると、
    VSCode で作成していないファイル(サードパーティのソースなど)まで
    誤って削除対象にしてしまう。
    """
    return rel.replace("/", "\\").startswith("..\\")


def plan_changes(disk: dict, previous: dict, registered: set):
    """追加・削除・リネームの対象を決める。

    削除の判定は「ディスクに無いのに登録されている」という現在の状態だけで決まる
    (マニフェストが無くても安全に動く)。
    リネームの判定だけ、前回実行時のハッシュ値との突き合わせを使う(履歴が無ければ
    単純な削除+追加として扱われる)。
    プロジェクトフォルダの外を指す登録(「既存の項目を追加」で持ち込んだファイルなど)は
    スキャン対象外なので、削除判定からは除外する。
    """
    to_remove = sorted(
        rel for rel in registered if rel not in disk and not is_outside_project_dir(rel)
    )
    to_add = sorted(rel for rel in disk if rel not in registered)

    renames = []
    remaining_removed = set(to_remove)
    remaining_added = set(to_add)
    for old_rel in to_remove:
        old_hash = previous.get(old_rel)
        if old_hash is None:
            continue
        match = next(
            (new_rel for new_rel in remaining_added if disk[new_rel]["hash"] == old_hash),
            None,
        )
        if match is not None:
            renames.append((old_rel, match))
            remaining_removed.discard(old_rel)
            remaining_added.discard(match)

    return sorted(remaining_added), sorted(remaining_removed), renames


def sync_project(name: str, dry_run: bool) -> None:
    project_dir = REPO_ROOT / name
    vcxproj = project_dir / f"{name}.vcxproj"
    filters = project_dir / f"{name}.vcxproj.filters"
    manifest_path = MANIFEST_DIR / f".vcxproj_sync_manifest.{name}.json"

    if not vcxproj.exists() or not filters.exists():
        print(f"[{name}] {vcxproj} または {filters} が見つかりません。")
        sys.exit(1)

    exclude_dir_names = BASE_EXCLUDE_DIR_NAMES | PROJECTS[name]

    disk = scan_disk(project_dir, exclude_dir_names)
    previous = load_manifest(manifest_path)

    proj = ProjectDoc(vcxproj, has_filters=False)
    filt = ProjectDoc(filters, has_filters=True)
    docs = (proj, filt)

    registered = set(proj.entries) | set(filt.entries)
    to_add, to_remove, renames = plan_changes(disk, previous, registered)

    for old_rel, new_rel in renames:
        kind = disk[new_rel]["kind"]
        for doc in docs:
            if old_rel in doc.entries:
                doc.rename(old_rel, new_rel)
            elif new_rel not in doc.entries:
                doc.add(new_rel, kind)

    for rel in to_add:
        kind = disk[rel]["kind"]
        for doc in docs:
            if rel not in doc.entries:
                doc.add(rel, kind)

    for rel in to_remove:
        for doc in docs:
            doc.remove(rel)

    changed = any(doc.dirty for doc in docs)

    print(f"[{name}]")
    print_summary(to_add, to_remove, renames, dry_run=dry_run)

    if dry_run:
        return

    if changed:
        for doc in docs:
            if doc.dirty:
                doc.write()

    save_manifest(manifest_path, disk)

    if changed:
        print("Visual Studio でプロジェクトを開き直す(または再読み込みする)と反映されます。")


def print_summary(to_add, to_remove, renames, dry_run: bool) -> None:
    if not (to_add or to_remove or renames):
        print("差分なし: 変更は必要ありませんでした。")
        return

    prefix = "[dry-run] " if dry_run else ""
    if renames:
        print(f"{prefix}リネーム/移動:")
        for old_rel, new_rel in renames:
            print(f"  {old_rel}  ->  {new_rel}")
    if to_add:
        print(f"{prefix}追加:")
        for rel in to_add:
            print(f"  {rel}")
    if to_remove:
        print(f"{prefix}削除:")
        for rel in to_remove:
            print(f"  {rel}")


def main() -> None:
    parser = argparse.ArgumentParser(
        description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter
    )
    parser.add_argument(
        "project", nargs="?", choices=sorted(PROJECTS), help="同期対象のプロジェクト名"
    )
    parser.add_argument(
        "--dry-run", action="store_true", help="変更内容を表示するだけで書き換えない"
    )
    parser.add_argument(
        "--list", action="store_true", help="対応プロジェクト一覧を表示して終了"
    )
    args = parser.parse_args()

    if args.list:
        print("対応プロジェクト:", ", ".join(sorted(PROJECTS)))
        return

    if not args.project:
        parser.error("project を指定してください(--list で一覧表示)。")

    sync_project(args.project, args.dry_run)


if __name__ == "__main__":
    main()
