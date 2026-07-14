/**
 * @file Light.h
 * @brief ライト関連
 */
#pragma once


namespace balloonEngineLow
{
	/**
	 * @brief ディレクションライト
	 * @note  定数バッファーとしてGPUへそのままコピーされるため、
	 *        仮想関数・継承・ポインタメンバーは禁止(PODを保つこと)。
	 *        メンバーの順番は model.fx の DirectionLight と完全に一致させること。
	 */
	struct DirectionLight
	{
		/** ライトの方向 */
		Vector3 lightDir;
		/** パディング(HLSLの16バイトアライメント合わせ) */
		float pad;
		/** ライトの色 */
		Vector4 lightColor;


		DirectionLight();
		~DirectionLight() = default;
	};




	/**********************************************/


	/**
	 * @brief ライトデータ(model.fx の LightCb と1:1対応)
	 * @note  ここを変更したらシェーダー側の cbuffer も必ず変更すること。
	 */
	struct LightData
	{
		/** ディレクションライト */
		DirectionLight directionLight;
		/** 環境光の色 */
		Vector4 ambientColor;
		/** 視点(カメラ)位置。鏡面反射で使用 */
		Vector3 eyePosition;
		/** パディング */
		float pad;


		LightData();
		~LightData() = default;
	};




	/**********************************************/


	/**
	 * @brief ライトマネージャー
	 */
	class LightManager : public Noncopyable
	{
		//=======================================================================
		// 更新
		//=======================================================================
	public:
		/**
		 * @brief 更新関数(毎フレーム、モデルの描画より前に1回呼ぶ)
		 * @note  視点位置をカメラから反映する。
		 */
		void Update();


		//=======================================================================
		// シーンライト
		//=======================================================================
	public:
		/**
		 * @brief ライトの方向を設定
		 * @param lightDir ライトの方向
		 */
		inline void SetLightDir(const Vector3& lightDir)
		{
			m_sceneLight.directionLight.lightDir.Set(lightDir);
		}
		/**
		 * @brief ライトの色を設定
		 * @param lightColor ライトの色
		 */
		inline void SetLightColor(const Vector4& lightColor)
		{
			m_sceneLight.directionLight.lightColor.Set(lightColor);
		}
		/**
		 * @brief アンビエントカラーを設定
		 * @param ambientColor アンビエントカラー
		 */
		inline void SetAmbientColor(const Vector4& ambientColor)
		{
			m_sceneLight.ambientColor.Set(ambientColor);
		}
		/**
		 * @brief シーンライトを取得(読み取り用)
		 * @return シーンライト
		 */
		inline const LightData& GetSceneLight() const
		{
			return m_sceneLight;
		}
		/**
		 * @brief シーンライトのアドレスを取得(定数バッファー登録用)
		 * @note  ModelRender::Init から m_expandConstantBuffer に渡すために使う。
		 * @return シーンライトのアドレス
		 */
		inline LightData* GetSceneLightAddress()
		{
			return &m_sceneLight;
		}


	private:
		/** シーンライト */
		LightData m_sceneLight;


		//=======================================================================
		// シングルトン
		//=======================================================================
	public:
		/**
		 * @brief ライトマネージャーを取得
		 * @return ライトマネージャー
		 */
		static LightManager& Get()
		{
			static LightManager instance;
			return instance;
		}

	private:
		LightManager() = default;
		~LightManager() = default;
	};

}