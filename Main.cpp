#define _USE_MATH_DEFINES

#include <cmath>
#include <iostream>
#include <memory>
#include <Windows.h>
#include <Mmdeviceapi.h>
#include <Audioclient.h>

using namespace std;

int main() {

	HRESULT result;

	// COMの初期化
	result = CoInitialize(NULL);
	if (FAILED(result)) {
		cout << "COMの初期化に失敗しました。" << endl;
		return -1;
	}

	// デバイス列挙子を取得
	IMMDeviceEnumerator* enumerator = nullptr;
	result = CoCreateInstance(__uuidof(MMDeviceEnumerator), NULL, CLSCTX_ALL, __uuidof(IMMDeviceEnumerator), (void**)&enumerator);
	if (FAILED(result)) {
		cout << "デバイス列挙子の取得に失敗しました。" << endl;
		return -1;
	}

	// オーディオデバイスを取得
	IMMDevice* device = nullptr;
	result = enumerator->GetDefaultAudioEndpoint(eRender, eConsole, &device);
	if (FAILED(result)) {
		cout << "オーディオデバイスの取得に失敗しました。" << endl;
		return -1;
	}

	// オーディオクライアントを取得
	IAudioClient* client = nullptr;
	result = device->Activate(__uuidof(IAudioClient), CLSCTX_ALL, NULL, (void**)&client);
	if (FAILED(result)) {
		cout << "オーディオクライアントの取得に失敗しました。" << endl;
		return -1;
	}

	// デバイスフォーマットを取得
	WAVEFORMATEX* format = nullptr;
	result = client->GetMixFormat(&format);
	if (FAILED(result)) {
		cout << "デバイスフォーマットの取得に失敗しました。" << endl;
		return -1;
	}

	// オーディオクライアントの初期化
	result = client->Initialize(AUDCLNT_SHAREMODE_SHARED, 0, 1 * 1000 * 1000 * 10, 0, format, NULL);
	if (FAILED(result)) {
		cout << "オーディオクライアントの初期化に失敗しました。" << endl;
		return -1;
	}

	// レンダークライアントを取得
	IAudioRenderClient* renderClient = nullptr;
	result = client->GetService(__uuidof(IAudioRenderClient), (void**)&renderClient);
	if (FAILED(result)) {
		cout << "レンダークライアントの初期化に失敗しました。" << endl;
		return -1;
	}

	// バッファサイズを取得
	uint32_t frameSize = 0;
	result = client->GetBufferSize(&frameSize);
	if (FAILED(result)) {
		cout << "バッファサイズの取得に失敗しました。" << endl;
		return -1;
	}

	// データをバッファに書き込み
	float* data = nullptr;
	result = renderClient->GetBuffer(frameSize, (BYTE**)&data);
	if (FAILED(result)) {
		cout << "バッファの取得に失敗しました。" << endl;
		return -1;
	}

	for (int i = 0; i < frameSize; ++i) {
		float value = 0.25f * sinf(2.0f * (float)M_PI * 1000.0f * i / format->nSamplesPerSec);
		data[2 * i + 0] = value;
		data[2 * i + 1] = value;
	}

	result = renderClient->ReleaseBuffer(frameSize, 0);
	if (FAILED(result)) {
		cout << "バッファの開放に失敗しました。" << endl;
		return -1;
	}

	client->Start();
	Sleep(1000);
	client->Stop();

	// デバイス類を破棄
	renderClient->Release();
	client->Release();
	device->Release();
	enumerator->Release();

	// COMの終了処理
	CoUninitialize();

	return 0;
}