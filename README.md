# ReadSndFile
## 基本仕様
SAEツールで使用される.sndファイルを読み取り、サウンドリスト等のデータを取得できます  

## 想定環境
Windows11  
C++言語標準：ISO C++17標準  

上記の環境で動作することを確認しています  
上記以外の環境での動作は保証しません  

## クラス/名前空間の概要
### class SAELib::SND
読み込んだSNDファイルのデータが格納される  
インスタンスを生成して使用する  

### class SAELib::SND::SoundData
格納されたデータのSoundパラメータを取得する際に使用するクラス  

### class SAELib::SNDConfig
ReadSndFileライブラリの動作設定が可能  
インスタンス生成不可  

### namespace SAELib::SNDError
本ライブラリが扱うエラー情報のまとめ  
throwされた例外をcatchするために使用する  

## クラス/名前空間の関数一覧
## class SAELib::SND
### デフォルトコンストラクタ
コンストラクタの引数を指定した場合、指定した引数でLoadSND関数を実行します  
引数を指定しない場合、ファイル読み込みは行いません  
```
SAELib::SND snd;
```
### 指定されたSNDファイルを読み込み
実行ファイルから子階層へファイル名を検索して読み込みます  
第二引数指定時は指定した階層からファイル名を検索します(SNDConfigよりも優先されます)  
実行時に既存の要素は初期化、上書きされます  
```
snd.LoadSND("kfm.snd");                 // 実行ファイルの階層から検索
snd.LoadSND("kfm.snd", "C:/MugenData"); // 指定パスから検索
```
引数1 const std::string& FileName ファイル名(拡張子 .snd は省略可)  
引数2 const std::string& FilePath 対象のパス(省略時は実行ファイルの子階層を探索)  
戻り値 bool 読み込み結果 (false = 失敗：true = 成功)

### 指定番号の存在確認
読み込んだSNDデータを検索し、指定番号が存在するかを確認します  
```
snd.ExistSoundNumber(5, 0); // サウンド番号5-0が存在するか確認
```
引数1 int32_t GroupNo グループ番号  
引数2 int32_t ItemNo アイテム番号  
戻り値 bool 検索結果 (false = 存在なし : true = 存在あり)

### 指定番号のデータへのアクセス
指定したサウンド番号のSNDデータへアクセスします  
対象が存在しない場合はSNDConfig::SetThrowErrorの設定に準拠します  
```
snd.GetSoundData(5, 0); // サウンド番号5-0のデータを取得
```
引数1 int32_t GroupNo グループ番号  
引数2 int32_t ItemNo アイテム番号  
戻り値1 対象が存在する GetSoundData(GroupNo, ItemNo)のデータ  
戻り値2 対象が存在しない SNDConfig::SetThrowError (false = ダミーデータの参照：true = 例外を投げる)  

### 指定インデックスデータの存在確認
読み込んだSNDデータを検索し、指定インデックスのデータ存在するかを確認します  
```
snd.ExistSoundDataIndex(0); // 0番目のデータが存在するか確認
```
引数1 int32_t index データ配列インデックス  
戻り値 bool 検索結果 (false = 存在なし : true = 存在あり)

### 指定インデックスのデータへアクセス
SNDデータへ指定したインデックスでアクセスします  
対象が存在しない場合はSNDConfig::SetThrowErrorの設定に準拠します  
```
snd.GetSoundDataIndex(0); // 0番目のデータを取得
```
引数1 int32_t index データ配列インデックス  
戻り値1 対象が存在する GetSoundDataIndex(index)のデータ  
戻り値2 対象が存在しない SNDConfig::SetThrowError (false = ダミーデータの参照：true = 例外を投げる)  



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
