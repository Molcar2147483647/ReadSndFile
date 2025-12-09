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

### SNDデータのサウンドグループ数を取得
読み込んだSNDデータのサウンドグループ数を返します  
```
snd.NumGroup(); // サウンドグループ数を取得
```
戻り値 int32_t NumGroup サウンドグループ数  

### SNDデータのサウンド数を取得
読み込んだSNDデータのサウンド数を返します  
```
snd.NumItem(); // サウンド数を取得
```
戻り値 int32_t NumItem サウンド数  

### SNDデータのファイル名を取得
読み込んだSNDデータの拡張子を除いたファイル名を返します  
```
snd.FileName(); // ファイル名を取得
```
戻り値 const std::string& FileName ファイル名  

### SNDデータの初期化
読み込んだSNDデータを初期化します  
```
snd.clear(); // SNDデータの初期化
```
戻り値 なし(void)  

### SNDデータの存在確認
読み込んだSNDデータの空かを判定します  
```
snd.empty(); // SNDデータの存在確認
```
戻り値 bool 判定結果 (false = データが存在：true = データが空)  

### SNDデータのデータサイズを取得
読み込んだSNDデータのデータサイズを返します  
```
air.size(); // SNDデータサイズを取得
```
戻り値 size_t SNDDataSize SNDデータサイズ 

## class SAELib::SND::SoundData
### ダミーデータ判断
自身がダミーデータであるかを確認します
SNDConfig::SetThrowError の設定が OFF の場合にエラー回避のために使用されます
```
snd.GetSoundData(XXX).IsDummy(); // ダミーデータ判断
```
戻り値 bool 判定結果 (false = 自身が正常なデータ：true = 自身がダミーデータ)

### グループ番号の取得
対象音声のグループ番号を返します
ダミーデータの場合は 0 を返します
```
snd.GetSoundData(XXX).GroupNo(); // グループ番号を取得
```
戻り値 int32_t GroupNo グループ番号

### アイテム番号の取得
対象音声のアイテム番号を返します
ダミーデータの場合は 0 を返します
```
snd.GetSoundData(XXX).ItemNo(); // アイテム番号を取得
```
戻り値 int32_t ItemNo アイテム番号

### バイトサイズの取得
対象音声データのバイトサイズを返します
ダミーデータの場合は 0 を返します
```
snd.GetSoundData(XXX).ByteSize(); // バイトサイズを取得
```
戻り値 int32_t ByteSize バイトサイズ

### チャンネル数の取得
対象音声のチャンネル数を返します
ダミーデータの場合は 0 を返します
```
snd.GetSoundData(XXX).Channel(); // チャンネル数を取得
```
戻り値 int32_t Channel チャンネル数

### ヘルツの取得
対象音声のヘルツを返します
ダミーデータの場合は 0 を返します
```
snd.GetSoundData(XXX).Hz(); // ヘルツを取得
```
戻り値 int32_t Hz ヘルツ

### ビット数の取得
対象音声のビット数を返します
ダミーデータの場合は 0 を返します
```
snd.GetSoundData(XXX).Bit(); // ビット数を取得
```
戻り値 int32_t Bit ビット数

### サンプル秒数の取得
対象音声のサンプル秒数を返します
ダミーデータの場合は 0 を返します
```
snd.GetSoundData(XXX).SampleRate(); // サンプル秒数を取得
```
戻り値 double SampleRate サンプル秒数

### フレーム秒数の取得
対象音声のフレーム秒数を返します
ダミーデータの場合は 0 を返します
```
snd.GetSoundData(XXX).SampleFrame(); // フレーム秒数を取得
```
戻り値 double SampleFrame フレーム秒数

### コメントデータの取得
SAEで設定された対象音声コメントを返します
ダミーデータの場合は DummyBinaryData を返します
DummyBinaryData は常に長さ 1 の配列で内容は `{0}` です
```
snd.GetSoundData(XXX).Comment(); // コメントデータを取得
```
戻り値1 const unsigned char* const Comment コメントデータ配列
戻り値2 const unsigned char* const DummyBinaryData ダミーデータ配列

## class SAELib::SNDConfig
### エラー出力切り替え設定/取得
このライブラリ関数で発生したエラーを例外として投げるかログとして記録するかを指定できます  
```
SAELib::SNDConfig::SetThrowError(bool flag); // エラー出力切り替え設定
```
引数1 bool (false = ログとして記録する：true = 例外を投げる)  
戻り値 なし(void)  
```
SAELib::SNDConfig::GetThrowError(); // エラー出力切り替え設定を取得
```
戻り値 bool (false = ログとして記録する：true = 例外を投げる)

### エラーログファイルを作成設定/取得
このライブラリ関数で発生したエラーのログファイルを出力するかどうか指定できます  
```
SAELib::SNDConfig::SetCreateLogFile(bool flag); // エラーログファイルを作成設定
```
引数1 bool (false = ログファイルを出力しない：true = ログファイルを出力する)  
戻り値 なし(void)  
```
SAELib::SNDConfig::GetCreateLogFile(); // エラーログファイルを作成設定を取得  
```
戻り値 bool (false = ログファイルを出力しない：true = ログファイルを出力する)    

### SAELibフォルダを作成設定/取得
ファイルの出力先としてSAELibファイルを使用するかを指定できます  
```
SAELib::SNDConfig::SetCreateSAELibFile(bool flag, const std::string& Path = ""); // SAELibフォルダを作成設定
```
引数1 bool (false = SAELibファイルを使用しない：true = SAELibファイルを使用する)  
引数2 const std::string& SAELibフォルダ作成先(省略時はパスの設定なし)  
戻り値 なし(void)  
```
SAELib::SNDConfig::GetCreateSAELibFile(); // SAELibフォルダを作成設定を取得  
```
戻り値 const std::string& CreateSAELibFile SAELibフォルダ作成先  

### SAELibフォルダのパス設定/取得
SAELibファイルの作成パスを指定できます  
```
SAELib::SNDConfig::SetSAELibFilePath(const std::string& Path = ""); // SAELibフォルダのパス設定
```
引数1 const std::string& SAELibFilePath SAELibフォルダ作成先  
戻り値 なし(void)  
```
SAELib::SNDConfig::GetSAELibFilePath(); // SAELibフォルダを作成パス取得  
```
戻り値 const std::string& SAELibFilePath SAELibフォルダ作成先  

### SNDファイルの検索パス設定/取得
SNDファイルの検索先のパスを指定できます  
SNDコンストラクタもしくはLoadSND関数で検索先のパスを指定しない場合、この設定のパスで検索します  
```
SAELib::SNDConfig::SetSNDSearchPath(const std::string& Path = ""); // SNDファイルの検索パス設定  
```
引数1 const std::string& SNDSearchPath SNDファイルの検索先のパス  
戻り値 なし(void)  
```
SAELib::SNDConfig::GetSNDSearchPath(); // SNDファイルの検索パス取得  
```
戻り値 const std::string& SNDSearchPath SNDファイルの検索先のパス  

## namespace SAELib::SNDError
### エラーID情報  
このライブラリが出力するエラーIDのenumです  
```
enum ErrorID : int32_t {
	NotFound_SNDFile,
	NotFound_SoundNumber,
	NotFound_SoundIndex,

	Invalid_SNDExtension,
	Invalid_LoadSNDPath,
	Invalid_SNDSearchPath,
	Invalid_EmptySNDFilePath,
	Invalid_SNDFileSize,
	Invalid_SNDSignature,
	Invalid_RIFFSignature,
	Invalid_WAVEFormat,
	Invalid_SAELibFolderPath,

	Failed_OpenSNDFile,
	Failed_CreateSAELibFolder,
	Failed_CreateErrorLogFile,
	Failed_WriteErrorLogFile,
	Failed_CloseErrorLogFile,
	Failed_CreateExportWAVFolder,
	Failed_CreateWAVFile,
	Failed_WriteWAVFile,
	Failed_CloseWAVFile,

	Corrupted_SNDFile,
	Warning_DuplicateSoundNumber,
};
```

### エラー情報配列
このライブラリが出力するエラー情報の配列です  
```
struct T_ErrorInfo {
public:
	const int32_t ID;
	const char* const Name;
	const char* const Message;
};
      
constexpr T_ErrorInfo ErrorInfo[] = {
	{ NotFound_SNDFile,				"NotFound_SNDFile",				"SNDファイルが見つかりません" },
	{ NotFound_SoundNumber,			"NotFound_SoundNumber",			"指定した番号がサウンドリストから見つかりません" },
	{ NotFound_SoundIndex,			"NotFound_SoundIndex",			"指定したインデックスがサウンドリストから見つかりません" },
			
	{ Invalid_SNDExtension,			"Invalid_SNDExtension",			"ファイルの拡張子が.sndではありません" },
	{ Invalid_LoadSNDPath,			"Invalid_LoadSNDPath",			"SNDファイル読み込み関数のパスが正しくありません" },
	{ Invalid_SNDSearchPath,		"Invalid_SNDSearchPath",		"SNDファイル検索フォルダのパスが正しくありません" },
	{ Invalid_EmptySNDFilePath,		"Invalid_EmptySNDFilePath",		"SNDファイルパスが指定されていません" },
	{ Invalid_SNDFileSize,			"Invalid_SNDFileSize",			"SNDファイルサイズが許容値を超えています" },
	{ Invalid_SNDSignature,			"Invalid_SNDSignature",			"ファイルの内部形式がSNDファイルではありません" },
	{ Invalid_RIFFSignature,		"Invalid_RIFFSignature",		"SNDファイル内の音声データがRIFF形式ではありません" },
	{ Invalid_WAVEFormat,			"Invalid_WAVEFormat",			"SNDファイル内の音声データのフォーマットがWAVE形式ではありません" },
	{ Invalid_SAELibFolderPath,		"Invalid_SAELibFolderPath",		"SAELibフォルダのパスが正しくありません" },
			
	{ Failed_OpenSNDFile,			"Failed_OpenSNDFile",			"SNDファイルが開けませんでした" },
	{ Failed_CreateSAELibFolder,	"Failed_CreateSAELibFolder",	"SAELibフォルダの作成に失敗しました" },
	{ Failed_CreateErrorLogFile,	"Failed_CreateErrorLogFile",	"エラーログファイルの作成に失敗しました" },
	{ Failed_WriteErrorLogFile,		"Failed_WriteErrorLogFile",		"エラーログファイルへの書き込みに失敗しました" },
	{ Failed_CloseErrorLogFile,		"Failed_CloseErrorLogFile",		"エラーログファイルへの書き込みが正常に終了しませんでした" },
	{ Failed_CreateExportWAVFolder,	"Failed_CreateExportWAVFolder",	"WAVファイル出力フォルダの作成に失敗しました" },
	{ Failed_CreateWAVFile,			"Failed_CreateWAVFile",			"WAVファイルの作成に失敗しました" },
	{ Failed_WriteWAVFile,			"Failed_WriteWAVFile",			"WAVファイルの書き込みに失敗しました" },
	{ Failed_CloseWAVFile,			"Failed_CloseWAVFile",			"WAVファイルの書き込みが正常に終了しませんでした" },
			
	{ Corrupted_SNDFile,			"Corrupted_SNDFile",			"SNDファイルが壊れている可能性があります" },
	{ Warning_DuplicateSoundNumber,	"Warning_DuplicateSoundNumber",	"サウンドリストの番号が重複しています" },
};

```

### エラー情報のサイズ取得
エラー情報の配列サイズを取得します  
```
SAELib::SNDError::ErrorInfoSize; // エラー情報配列サイズ
```
戻り値 size_t ErrorInfoSize エラー情報配列サイズ

### エラー名取得
エラーIDに応じたエラー名を取得します  
```
SAELib::SNDError::ErrorName(ErrorID); // ErrorIDのエラー名を取得
```
引数1 int32_t ErrorID エラーID  
戻り値 const char* ErrorName エラー名  

### エラーメッセージ取得
エラーIDに応じたエラーメッセージを取得します  
```
SAELib::SNDError::ErrorMessage(ErrorID); // ErrorIDのエラーメッセージを取得
```
引数1 int32_t ErrorID エラーID  
戻り値 const char* ErrorMessage エラーメッセージ  

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
