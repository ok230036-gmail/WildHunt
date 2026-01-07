#pragma once
#include <wrl/client.h>

// XAUDIO2.9
#include <xaudio2.h>
#include <xaudio2fx.h>
#include "WAVFileReader.h"		// Microsfotの公式チュートリアルで配布しているWAVファイル読み込みライブラリ
#include <list>
#include <vector>
#include <memory>

#pragma comment(lib, "xaudio2.lib")

using namespace std;
using Microsoft::WRL::ComPtr;

#define MAX_SOUNDS	32

class SoundContainer
{
private:
	std::unique_ptr<uint8_t[]>	m_waveFile;
	DirectX::WAVData			m_wavData;

public:
	SoundContainer(const WCHAR* fileName);

	DirectX::WAVData* GetWavData()
	{
		return &m_wavData;
	}
};

class PlayingSoundContainer
{
private:
	IXAudio2SourceVoice* voice;
	IXAudio2SubmixVoice* submix;
	XAUDIO2_BUFFER		buf;

	bool				m_bPlaying;

public:
	PlayingSoundContainer()
	{
		voice = nullptr;
		submix = nullptr;
		buf = {};

		m_bPlaying = false;
	}

	HRESULT PlayWavData(ComPtr<IXAudio2>& pXAudio2, DirectX::WAVData* wavData);
	void Stop();
	void ReleaseWavData();

	void Update();

	bool IsFinished()
	{
		return !m_bPlaying;
	}

	IXAudio2SourceVoice* GetPSourceVoice()
	{
		return voice;
	}

	IXAudio2SubmixVoice* GetPSubMix()
	{
		return submix;
	}

	XAUDIO2_BUFFER* GetPAudioBuffer()
	{
		return &buf;
	}

	~PlayingSoundContainer();
};

class SoundManager
{
private:
	ComPtr<IXAudio2> m_pXaudio2;
	IXAudio2MasteringVoice* m_pMasteringVoice;
	
	vector<unique_ptr<SoundContainer>> m_soundContainers;
	vector<unique_ptr<PlayingSoundContainer>> m_playingContainers;

	int	m_seCount;
	int m_playIndex;

public:
	SoundManager();
	~SoundManager();

	void AudioUpdate();

	HRESULT InitSoundManager();
	void DestructSoundManager();

	bool LoadSoundFile(const WCHAR* fileName, int& soundId);
	bool DeleteSoundFile(int soundId);

	int Play(UINT soundId);
	void Stop(UINT playingId);
};