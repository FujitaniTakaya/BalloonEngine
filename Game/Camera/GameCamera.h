/**
 * @file GameCamera.h
 * @brief ゲームカメラクラスの宣言
 */


namespace app
{
    namespace camera
    {
        /**
         * @brief ゲームカメラクラス
         */
        class GameCamera : public Noncopyable
        {
        public:
            GameCamera();
            ~GameCamera();


        public:
            void Start();
            void Update();


        public:
            void SetTargetPosition(const Vector3& position);


        public:
        private:
            /** 注視点から視点までのベクトル */
            Vector3 m_toCameraPos;
            /** ターゲットのポジション */
            Vector3 m_targetPosition;
        };
    } // namespace camera
} // namespace app