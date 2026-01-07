#include "SoundManager.h"
#include <algorithm>

SoundManager::SoundManager()
{
	m_seCount = 0;
	m_playIndex = 0;
	m_pMasteringVoice = nullptr;

	for (int i = 0; i < MAX_SOUNDS; i++)
	{
		// 発音リスト作成
		m_playingContainers.push_back(make_unique<PlayingSoundContainer>());
	}
}

SoundManager::~SoundManager()
{
	m_soundContainers.clear();
	m_playingContainers.clear();

	if (m_pMasteringVoice)
	{
		m_pMasteringVoice->DestroyVoice();
		m_pMasteringVoice = nullptr;
	}
	m_pXaudio2.Reset();					// ResetはComPtr側のメソッド。
}

bool SoundManager::LoadSoundFile(const WCHAR* fileName, int& soundId)
{
	unique_ptr<SoundContainer> seCon(new SoundContainer(fileName), default_delete<SoundContainer>());

	m_soundContainers.push_back(std::move(seCon));
	soundId = m_seCount;
	m_seCount++;
	return true;
}

bool SoundManager::DeleteSoundFile(int soundId)
{
	return false;
}

int SoundManager::Play(UINT soundId)
{
	int index = -1;

	// index検索
	for (int i = m_playIndex; i < MAX_SOUNDS; i++)
	{
		if (m_playingContainers[i]->IsFinished())
		{
			index = i;
			break;
		}
	}

	if (index < 0)
	{
		for (int i = 0; i < m_playIndex; i++)
		{
			if (m_playingContainers[i]->IsFinished())
			{
				index = i;
				break;
			}
		}
	}

	if (index >= 0)
	{
		// 空きスロットがあった
		if (SUCCEEDED(m_playingContainers[index]->PlayWavData(m_pXaudio2, m_soundContainers[soundId]->GetWavData()) ) )
		{
			m_playIndex = (index + 1) % MAX_SOUNDS;
		}
	}

	return index;
}

void SoundManager::Stop(UINT playingId)
{
	if (playingId < MAX_SOUNDS)
	{
		m_playingContainers[playingId]->Stop();
	}
}

void SoundManager::AudioUpdate()
{
	for (int i = 0; i < MAX_SOUNDS; i++)
	{
		m_playingContainers[i]->Update();
	}
}

HRESULT SoundManager::InitSoundManager()
{
	HRESULT hr;

	hr = XAudio2Create(m_pXaudio2.GetAddressOf(), 0);
	if (FAILED(hr))
	{
		m_pXaudio2 = nullptr;
		return hr;
	}
	hr = m_pXaudio2->CreateMasteringVoice(&m_pMasteringVoice);
	if (FAILED(hr))
	{
		m_pMasteringVoice = nullptr;
		m_pXaudio2 = nullptr;
		return hr;
	}

	XAUDIO2FX_REVERB_I3DL2_PARAMETERS xau2fx_i3d = XAUDIO2FX_I3DL2_PRESET_STONECORRIDOR;
	XAUDIO2FX_REVERB_PARAMETERS reverb_param = { 0 };
	ReverbConvertI3DL2ToNative(&xau2fx_i3d, &reverb_param);

	return hr;
}

void SoundManager::DestructSoundManager()
{
	m_pMasteringVoice = nullptr;
	m_pXaudio2 = nullptr;
}

SoundContainer::SoundContainer(const WCHAR* fileName)
{
	HRESULT hr;
	hr = DirectX::LoadWAVAudioFromFileEx(fileName, m_waveFile, m_wavData);

	if (FAILED(hr))
	{
		m_wavData.audioBytes = 0;
	}
}

HRESULT PlayingSoundContainer::PlayWavData(ComPtr<IXAudio2>& pXAudio2, DirectX::WAVData* wavData)
{
	HRESULT hr = E_FAIL;
	if (wavData->audioBytes > 0)
	{
		// データがある時のみ
		ReleaseWavData();

		hr = pXAudio2->CreateSubmixVoice(&submix, wavData->wfx->nChannels, wavData->wfx->nSamplesPerSec, 0, 0, 0, 0);
		if (FAILED(hr))
		{
			submix = nullptr;
			return hr;
		}

		hr = pXAudio2->CreateSourceVoice(&voice, wavData->wfx,
			0, XAUDIO2_DEFAULT_FREQ_RATIO, NULL, NULL, NULL);

		if (FAILED(hr))
		{
			// 起動失敗
			submix->DestroyVoice();
			submix = nullptr;

			voice = nullptr;
			return hr;
		}


		// XAUDIOBUFFER設定
		buf.AudioBytes = wavData->audioBytes;
		buf.LoopBegin = wavData->loopStart;
		buf.LoopLength = wavData->loopLength;
		buf.pAudioData = wavData->startAudio;
		buf.pContext = 0;
		buf.PlayBegin = 0;
		buf.Flags = XAUDIO2_END_OF_STREAM;
		buf.PlayLength = 0;

		if (buf.LoopLength == 0)
			buf.LoopCount = 0;
		else
			buf.LoopCount = XAUDIO2_LOOP_INFINITE;

		// AUDIOBUFFERをコミット(サウンドカードへ転送)
		voice->SubmitSourceBuffer(&buf);

		hr = voice->Start(0);

		if (SUCCEEDED(hr))
		{
			// 再生中フラグON
			m_bPlaying = true;
		}
	}

	return hr;
}

void PlayingSoundContainer::Stop()
{
	if (m_bPlaying && voice)
	{
		// 再生中フラグOFF
		m_bPlaying = false;
		voice->Stop();
	}
}

void PlayingSoundContainer::ReleaseWavData()
{
	if (voice)
	{
		submix->DestroyVoice();
		voice->DestroyVoice();

		submix = nullptr;
		voice = nullptr;

		// 再生中フラグOFF
		m_bPlaying = false;
	}
}

void PlayingSoundContainer::Update()
{
	XAUDIO2_VOICE_STATE state;

	if (voice && m_bPlaying)
	{
		voice->GetState(&state);
		if (state.BuffersQueued < 1)
		{
			m_bPlaying = false;
		}
	}
}

PlayingSoundContainer::~PlayingSoundContainer()
{
	Stop();
	ReleaseWavData();
}
