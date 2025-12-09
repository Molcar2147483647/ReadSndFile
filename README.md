# ReadSndFile




## 使用例
```
#include "h_ReadSndFile.h"
#include <iostream> // 標準入力/出力

int main()
{
	SAELib::SNDConfig::SetThrowError(false);		// このライブラリで発生したエラーを例外として処理しない
	SAELib::SNDConfig::SetCreateSAELibFile(false);	// SAELibファイルの作成を許可する
	SAELib::SNDConfig::SetSAELibFilePath();			// SAELibファイルの作成階層を指定
	SAELib::SNDConfig::SetCreateLogFile(false);		// このライブラリで発生したエラーログの作成を許可する
	SAELib::SNDConfig::SetSNDSearchPath("../../");	// sndファイルの検索開始階層を指定

	// kfmのsndファイルを読み込む
	SAELib::SND snd("M16_Boss");

	// サウンド番号5-0が存在するか確認
	if (snd.ExistSoundNumber(5, 0)) {
		// サウンド番号5-0の音声番号を取得
		snd.GetSoundData(5, 0).GroupNo();
		snd.GetSoundData(5, 0).ItemNo();
	}

	// すべてのサウンド情報を出力
	for (int32_t SoundIndex = 0; snd.NumItem() > SoundIndex; ++SoundIndex) {
		std::cout << "\nExist: " << snd.ExistSoundDataIndex(SoundIndex) << std::endl;
		std::cout << "GroupNo: " << snd.GetSoundDataIndex(SoundIndex).GroupNo() << std::endl;
		std::cout << "ItemNo: " << snd.GetSoundDataIndex(SoundIndex).ItemNo() << std::endl;
		std::cout << "ByteSize: " << snd.GetSoundDataIndex(SoundIndex).ByteSize() << std::endl;
		std::cout << "Channel: " << snd.GetSoundDataIndex(SoundIndex).Channel() << std::endl;
		std::cout << "Hertz: " << snd.GetSoundDataIndex(SoundIndex).Hz() << std::endl;
		std::cout << "Bit: " << snd.GetSoundDataIndex(SoundIndex).Bit() << std::endl;
		std::cout << "SampleRate: " << snd.GetSoundDataIndex(SoundIndex).SampleRate() << std::endl;
		std::cout << "SampleFrame: " << snd.GetSoundDataIndex(SoundIndex).SampleFrame() << std::endl;
		std::cout << "Comment: " << snd.GetSoundDataIndex(SoundIndex).Comment() << std::endl;
	}

	// 存在しない番号はダミーデータに
	std::cout << snd.GetSoundData(-999, -999).IsDummy() << std::endl;

	return 0;
}
```
