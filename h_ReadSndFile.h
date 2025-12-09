#ifndef INCLUDEGUARD_READSNDFILE_HEADER
#define INCLUDEGUARD_READSNDFILE_HEADER

#include <stdint.h>			// uint32_tとかのやつ
#include <fstream>			// ファイル読み取り
#include <filesystem>		// ファイル検索
#include <vector>			// 可変長配列
#include <unordered_map>	// ハッシュ的なやつ
#include <type_traits>		// std::enable_ifのやつ

namespace SAELib {
	namespace ReadSndFile_detail {
		using ksize_t = uint32_t;
		inline constexpr ksize_t KSIZE_MAX = static_cast<ksize_t>(~ksize_t{ 0 });

		namespace ReadSndFileFormat {
			inline constexpr double kVersion = 1.00;
			inline constexpr std::string_view kSystemDirectoryName = "SAELib";
			inline constexpr std::string_view kErrorLogFileName = "SAELib_SndErrorLog";

			inline constexpr unsigned char kDummyBinaryArray[1] = { 0 };
			inline constexpr const unsigned char* kDummyBinaryData = kDummyBinaryArray;
			inline constexpr std::string_view kDummyStringView = "";
		};

		namespace SNDFormat {
			inline constexpr std::string_view kExtension = ".snd";
			inline constexpr std::string_view kSignature = "ElecbyteSnd";
			inline constexpr uint32_t kSNDVersion = 0x01000100;
			inline constexpr uint32_t kSNDVersion2 = 0x00010001;
			inline constexpr ksize_t kSubHeaderStart = 512;
			inline constexpr ksize_t kFileSizeLimit = 0xffffffff;
		};

		namespace DecodeEndian {
			[[nodiscard]] inline constexpr uint16_t UInt16LE(const unsigned char* const buffer) noexcept {
				return buffer[0] | (buffer[1] << 8);
			}
			[[nodiscard]] inline constexpr uint32_t UInt32LE(const unsigned char* const buffer) noexcept {
				return buffer[0] | (buffer[1] << 8) | (buffer[2] << 16) | (buffer[3] << 24);
			}
			[[nodiscard]] inline constexpr uint16_t UInt16BE(const unsigned char* const buffer) noexcept {
				return buffer[1] | (buffer[0] << 8);
			}
			[[nodiscard]] inline constexpr uint32_t UInt32BE(const unsigned char* const buffer) noexcept { //（未使用）
				return buffer[3] | (buffer[2] << 8) | (buffer[1] << 16) | (buffer[0] << 24);
			}
		};

		template<typename T, typename = std::enable_if_t<std::is_same_v<T, int32_t> || std::is_same_v<T, int64_t>>>
		struct T_Bit {
		private:
			const T kBitStart;
			const T kBitRange;
		public:
			constexpr T_Bit(int32_t BitStart, T BitRange) : kBitStart(BitStart), kBitRange(BitRange) {}
			[[nodiscard]] constexpr T BitStart() const noexcept { return kBitStart; }
			[[nodiscard]] constexpr T BitRange() const noexcept { return kBitRange; }
			[[nodiscard]] constexpr T BitMask() const noexcept { return kBitRange << kBitStart; }
			[[nodiscard]] constexpr T BitSet(T Value) const noexcept { return (Value & kBitRange) << kBitStart; }
			[[nodiscard]] constexpr T BitGet(T Value) const noexcept { return (Value >> kBitStart) & kBitRange; }
		};

		struct Convert {
		private:
			inline static constexpr T_Bit<int64_t> kSoundGroupNo = T_Bit<int64_t>(0, UINT32_MAX);
			inline static constexpr T_Bit<int64_t> kSoundItemNo = T_Bit<int64_t>(32, UINT32_MAX);

		public:
			[[nodiscard]] inline static constexpr int64_t EncodeSoundNumber(int32_t GroupNo, int32_t ItemNo) noexcept {
				return kSoundGroupNo.BitSet(GroupNo) | kSoundItemNo.BitSet(ItemNo);
			}

			[[nodiscard]] inline static constexpr int32_t DecodeSoundGroupNo(int64_t SoundNumber) noexcept { return static_cast<int32_t>(kSoundGroupNo.BitGet(SoundNumber)); }
			[[nodiscard]] inline static constexpr int32_t DecodeSoundItemNo(int64_t SoundNumber) noexcept { return static_cast<int32_t>(kSoundItemNo.BitGet(SoundNumber)); }
		};

		struct T_Config {
		private:
			T_Config() = default;
			~T_Config() = default;
			T_Config(const T_Config&) = delete;
			T_Config& operator=(const T_Config&) = delete;

		private:
			int32_t BitFlag_ = {};
			// ビットフラグ設定:
			// bit 0 (0x01): このライブラリが例外を投げるか
			// bit 1 (0x02): エラーログファイルを生成するか
			// bit 2 (0x04): SAELibファイルを作成するか
			// 
			// SAELibファイルの生成パス指定
			// SNDデータ検索開始ディレクトリパス指定
			// 

			inline static constexpr int32_t kThrowError = 1 << 0;
			inline static constexpr int32_t kCreateLogFile = 1 << 1;
			inline static constexpr int32_t kCreateSAELibFile = 1 << 2;
			inline static constexpr int32_t kDefaultConfig = 0;

			// SAELibファイルのパス
			std::filesystem::path SAELibFilePath_ = {};

			// SNDファイル検索開始パス
			std::filesystem::path SNDSearchPath_ = {};

		public:
			[[nodiscard]] static T_Config& Instance() {
				static T_Config instance;
				return instance;
			}

		public:
			[[nodiscard]] int32_t BitFlag() const noexcept { return BitFlag_; }
			[[nodiscard]] bool ThrowError() const noexcept { return (BitFlag_ & kThrowError) != 0; }
			[[nodiscard]] bool CreateLogFile() const noexcept { return (BitFlag_ & kCreateLogFile) != 0; }
			[[nodiscard]] bool CreateSAELibFile() const noexcept { return (BitFlag_ & kCreateSAELibFile) != 0; }
			[[nodiscard]] const std::filesystem::path& SAELibFilePath() const noexcept { return SAELibFilePath_; }
			[[nodiscard]] const std::filesystem::path& SNDSearchPath() const noexcept { return SNDSearchPath_; }

			void InitConfig() { BitFlag_ = kDefaultConfig; }
			void ThrowError(bool flag) { BitFlag_ = (BitFlag_ & ~kThrowError) | (flag ? kThrowError : 0); }
			void CreateLogFile(bool flag) { BitFlag_ = (BitFlag_ & ~kCreateLogFile) | (flag ? kCreateLogFile : 0); }
			void CreateSAELibFile(bool flag) { BitFlag_ = (BitFlag_ & ~kCreateSAELibFile) | (flag ? kCreateSAELibFile : 0); }
			void SAELibFilePath(const std::filesystem::path& Path) { SAELibFilePath_ = (Path.empty() ? std::filesystem::current_path() : Path); }
			void SNDSearchPath(const std::filesystem::path& Path) { SNDSearchPath_ = (Path.empty() ? std::filesystem::current_path() : Path); }
		};

		// ユーザーが参照する範囲なのでkプレフィックスはなしで

		/**
		* @brief エラー情報
		*
		* 　このライブラリが使用するエラー情報の名前空間です
		*/
		namespace ErrorMessage {

			/**
			* @brief エラー情報構造体
			*
			* 　このライブラリが出力するエラー情報の構造体です
			*
			* @param const int32_t ID
			* @param const char* const Name
			* @param const char* const Message
			*/
			struct T_ErrorInfo {
			public:
				const int32_t ID;
				const char* const Name;
				const char* const Message;
			};

			/**
			* @brief エラーID情報
			*
			* 　このライブラリが出力するエラーIDのenumです
			*
			* @return enum ErrorID エラーID
			*/
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

			/**
			* @brief エラー情報配列
			*
			* 　このライブラリが出力するエラー情報の配列です
			*
			* @return T_ErrorInfo ErrorInfo エラー情報
			*/
			inline constexpr T_ErrorInfo ErrorInfo[] = {
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

			/**
			* @brief エラー情報のサイズ取得
			*
			* 　エラー情報の配列サイズを取得します
			*
			* @return size_t ErrorInfoSize エラー情報配列サイズ
			*/
			inline constexpr size_t ErrorInfoSize = sizeof(ErrorInfo) / sizeof(ErrorInfo[0]);

			/**
			* @brief エラー名の取得
			*
			* 　エラーIDに応じたエラー名を取得します
			*
			* @param int32_t ErrorID エラーID
			* @return const char* ErrorName エラー名
			*/
			inline constexpr const char* ErrorName(int32_t ID) { return ErrorInfo[ID].Name; }

			/**
			* @brief エラーメッセージの取得
			*
			* 　エラーIDに応じたエラーメッセージを取得します
			*
			* @param int32_t ErrorID エラーID
			* @return const char* ErrorMessage エラーメッセージ
			*/
			inline constexpr const char* ErrorMessage(int32_t ID) { return ErrorInfo[ID].Message; }
		}

		struct T_ErrorHandle {
		private:
			T_ErrorHandle() = default;
			~T_ErrorHandle() = default;
			T_ErrorHandle(const T_ErrorHandle&) = delete;
			T_ErrorHandle& operator=(const T_ErrorHandle&) = delete;

		private:
			struct T_ErrorList {
			private:
				const int32_t kErrorID;
				const int32_t kErrorValue = 0;
				const int32_t kErrorValue2 = 0;

			public:
				[[nodiscard]] int32_t ErrorID() const noexcept { return kErrorID; }
				[[nodiscard]] int32_t ErrorValue() const noexcept { return kErrorValue; }
				[[nodiscard]] int32_t ErrorValue2() const noexcept { return kErrorValue2; }
				[[nodiscard]] const char* const ErrorName() const noexcept { return ErrorMessage::ErrorInfo[kErrorID].Name; }
				[[nodiscard]] const char* const ErrorMessage() const noexcept { return ErrorMessage::ErrorInfo[kErrorID].Message; }

				T_ErrorList(int32_t ErrorID)
					: kErrorID(ErrorID) {
				}

				T_ErrorList(int32_t ErrorID, int32_t ErrorValue)
					: kErrorID(ErrorID), kErrorValue(ErrorValue) {
				}

				T_ErrorList(int32_t ErrorID, int32_t GroupNo, int32_t ItemNo)
					: kErrorID(ErrorID), kErrorValue(GroupNo), kErrorValue2(ItemNo) {
				}
			};
			std::vector<T_ErrorList> ErrorList = {};

			void InitErrorList() {
				ErrorList.clear();
			}

		public:
			[[nodiscard]] static T_ErrorHandle& Instance() {
				static T_ErrorHandle instance;
				return instance;
			}

		public:
			void AddErrorList(int32_t ErrorID) {
				ErrorList.emplace_back(T_ErrorList(ErrorID));
			}

			void AddErrorList(int32_t ErrorID, int32_t ErrorValue) {
				ErrorList.emplace_back(T_ErrorList(ErrorID, ErrorValue));
			}

			void AddErrorList(int32_t ErrorID, int32_t GroupNo, int32_t ItemNo) {
				ErrorList.emplace_back(T_ErrorList(ErrorID, GroupNo, ItemNo));
			}

			[[noreturn]] void ThrowError(int32_t ErrorID) const {
				throw std::runtime_error(ErrorMessage::ErrorInfo[ErrorID].Name);
			}

			[[noreturn]] void ThrowError(int32_t ErrorID, int32_t ErrorValue) const {
				throw std::runtime_error(ErrorMessage::ErrorInfo[ErrorID].Name);
			}

			[[noreturn]] void ThrowError(int32_t ErrorID, int32_t GroupNo, int32_t ItemNo) const {
				throw std::runtime_error(ErrorMessage::ErrorInfo[ErrorID].Name);
			}

			void SetError(int32_t ErrorID) {
				if (!T_Config::Instance().ThrowError()) {
					AddErrorList(ErrorID);
					return;
				}
				ThrowError(ErrorID);
			}

			void SetError(int32_t ErrorID, int32_t ErrorValue) {
				if (!T_Config::Instance().ThrowError()) {
					AddErrorList(ErrorID, ErrorValue);
					return;
				}
				ThrowError(ErrorID, ErrorValue);
			}

			void SetError(int32_t ErrorID, int32_t GroupNo, int32_t ItemNo) {
				if (!T_Config::Instance().ThrowError()) {
					AddErrorList(ErrorID, GroupNo, ItemNo);
					return;
				}
				ThrowError(ErrorID, GroupNo, ItemNo);
			}

			void WriteErrorLog(std::ofstream& File) {
				File << "ReadSndFile ErrorLog" << "\n";
				File << "エラー数: " << ErrorList.size() << "\n";

				for (auto& Error : ErrorList) {
					File << "\nエラー名: " << Error.ErrorName() << "\n";
					File << "エラー内容: " << Error.ErrorMessage() << "\n";
					if (Error.ErrorID() == ErrorMessage::Warning_DuplicateSoundNumber || Error.ErrorID() == ErrorMessage::NotFound_SoundNumber) {
						File << "エラー値: " << Error.ErrorValue() << "-" << Error.ErrorValue2() << "\n";
					}
					if (Error.ErrorID() == ErrorMessage::NotFound_SoundIndex) {
						File << "エラー値: " << Error.ErrorValue() << "\n";
					}
				}
				File.flush();

				if (File.fail() || File.bad()) {
					if (T_Config::Instance().ThrowError()) {
						ThrowError(ErrorMessage::Failed_WriteErrorLogFile);
					}
				}
				File.close();
				if (File.fail() || File.bad()) {
					if (T_Config::Instance().ThrowError()) {
						ThrowError(ErrorMessage::Failed_CloseErrorLogFile);
					}
				}
			}
		};

		// パスを扱う時の補助
		struct T_FilePathSystem {
		private:
			std::filesystem::path Path_ = {};
			std::error_code ErrorCode_ = {};

		public:
			const std::filesystem::path& Path() const noexcept { return Path_; }
			const std::error_code& ErrorCode() const noexcept { return ErrorCode_; }

			void SetPath(const std::filesystem::path& Path) {
				ErrorCode_.clear();
				Path_ = weakly_canonical(std::filesystem::absolute(Path), ErrorCode_);
				if (ErrorCode_) { Path_.clear(); }
			}

			void CreateDirectory(const std::filesystem::path& Path) {
				ErrorCode_.clear();
				if (std::filesystem::exists(Path)) { return; }
				std::filesystem::create_directories(Path, ErrorCode_);

			}

		public:
			T_FilePathSystem() = default;

			T_FilePathSystem(const std::filesystem::path& Path) {
				SetPath(Path);
			}

			[[nodiscard]] bool empty() const noexcept { return Path_.empty(); }
		};
		
		// サウンドリストのサウンド番号の重複チェック＆存在確認
		template<typename T, typename = std::enable_if_t<std::is_same_v<T, int32_t> || std::is_same_v<T, int64_t>>>
		struct T_UnorderedMap {
		private:
			std::unordered_map<T, int32_t> UnorderedMap = {};

		public:
			void Register(T value) {
				UnorderedMap[value] = static_cast<int32_t>(UnorderedMap.size());
			}

			void Register(int32_t value1, int32_t value2) {
				UnorderedMap[Convert::EncodeSoundNumber(value1, value2)] = static_cast<int32_t>(UnorderedMap.size());
			}

		public:
			T_UnorderedMap() = default;

			[[nodiscard]] int32_t find(T input) {
				auto it = UnorderedMap.find(input);
				if (it != UnorderedMap.end()) { return it->second; }
				return -1;
			}
			[[nodiscard]] int32_t find(int32_t value1, int32_t value2) {
				return find(Convert::EncodeSoundNumber(value1, value2));
			}

			[[nodiscard]] bool exist(T value) {
				return find(value) >= 0;
			}
			[[nodiscard]] bool exist(int32_t value1, int32_t value2) {
				return find(value1, value2) >= 0;
			}

			void reserve(ksize_t value) {
				UnorderedMap.reserve(value);
			}

			void clear() {
				UnorderedMap.clear();
			}

			void shrink_to_fit() {
				UnorderedMap.rehash(0);
			}

			[[nodiscard]] bool empty() const noexcept {
				return UnorderedMap.empty();
			}

			[[nodiscard]] ksize_t size() const noexcept {
				return static_cast<ksize_t>(UnorderedMap.size());
			}
		};

		struct T_ReadWAVEBinary {
		private:
			const std::vector<unsigned char>& kWAVEVector;
			const ksize_t kSoundStart;
			const ksize_t kSoundSize;
			int32_t fmtChunkOffset = 0;
			int32_t factChunkOffset = 0;
			int32_t dataChunkOffset = 0;
			int32_t SAECChunkOffset = 0;

			inline static constexpr int32_t kRIFFChunkOffset = 0;
			inline static constexpr int32_t kRIFFChunkDataSize = 12;

			[[nodiscard]] const unsigned char* WAVEBinary() const noexcept { return kWAVEVector.data() + kSoundStart; }

			struct WAVEFormat {
				inline static constexpr std::string_view kRIFFSignature = "RIFF";
				inline static constexpr std::string_view kFormatSignature = "WAVE";
				inline static constexpr std::string_view kfmtSignature = "fmt ";
				inline static constexpr std::string_view kfactSignature = "fact";
				inline static constexpr std::string_view kdataSignature = "data";
				inline static constexpr std::string_view kSAECSignature = "SAEC";
			};

			struct T_WAVERIFFChunk {
			private:
				const unsigned char* const ChunkData;

			public:
				[[nodiscard]] std::string_view Signature() const noexcept { return std::string_view(reinterpret_cast<const char*>(ChunkData), WAVEFormat::kRIFFSignature.size()); }
				[[nodiscard]] uint32_t ChunkSize() const noexcept { return DecodeEndian::UInt32LE(&ChunkData[4]); }
				[[nodiscard]] std::string_view FormatSignature() const noexcept { return std::string_view(reinterpret_cast<const char*>(&ChunkData[8]), WAVEFormat::kFormatSignature.size()); }

				T_WAVERIFFChunk(const unsigned char* const ChunkData) : ChunkData(ChunkData) {}
			};

			struct T_WAVEfmtChunk {
			private:
				const unsigned char* const ChunkData;

			public:
				[[nodiscard]] std::string_view Signature() const noexcept { return std::string_view(reinterpret_cast<const char*>(ChunkData), WAVEFormat::kfmtSignature.size()); }
				[[nodiscard]] uint32_t ChunkSize() const noexcept { return DecodeEndian::UInt32LE(&ChunkData[4]); }

				[[nodiscard]] uint16_t FormatTag() const noexcept { return DecodeEndian::UInt16LE(&ChunkData[8]); }
				[[nodiscard]] uint16_t Channels() const noexcept { return DecodeEndian::UInt16LE(&ChunkData[10]); }
				[[nodiscard]] uint32_t SamplesPerSec() const noexcept { return DecodeEndian::UInt32LE(&ChunkData[12]); }
				[[nodiscard]] uint32_t AvgBytesPerSec() const noexcept { return DecodeEndian::UInt32LE(&ChunkData[16]); }
				[[nodiscard]] uint16_t BlockAlign() const noexcept { return DecodeEndian::UInt16LE(&ChunkData[20]); }
				[[nodiscard]] uint16_t BitsPerSample() const noexcept { return DecodeEndian::UInt16LE(&ChunkData[22]); }
				[[nodiscard]] uint16_t cbSize() const noexcept { return DecodeEndian::UInt16LE(&ChunkData[24]); }

				T_WAVEfmtChunk(const unsigned char* const ChunkData) : ChunkData(ChunkData) {}
			};

			struct T_WAVEfactChunk {
			private:
				const unsigned char* const ChunkData;

			public:
				[[nodiscard]] std::string_view Signature() const noexcept { return std::string_view(reinterpret_cast<const char*>(ChunkData), WAVEFormat::kfactSignature.size()); }
				[[nodiscard]] uint32_t ChunkSize() const noexcept { return DecodeEndian::UInt32LE(&ChunkData[4]); }
				[[nodiscard]] uint32_t SampleLength() const noexcept { return DecodeEndian::UInt32LE(&ChunkData[8]); }

				T_WAVEfactChunk(const unsigned char* const ChunkData) : ChunkData(ChunkData) {}
			};

			struct T_WAVEdataChunk {
			private:
				const unsigned char* const ChunkData;

			public:
				[[nodiscard]] std::string_view Signature() const noexcept { return std::string_view(reinterpret_cast<const char*>(ChunkData), WAVEFormat::kdataSignature.size()); }
				[[nodiscard]] uint32_t ChunkSize() const noexcept { return DecodeEndian::UInt32LE(&ChunkData[4]); }

				T_WAVEdataChunk(const unsigned char* const ChunkData) : ChunkData(ChunkData) {}
			};

			struct T_WAVESAECChunk {
			private:
				const unsigned char* const ChunkData;

			public:
				[[nodiscard]] bool IsDummy() const noexcept { return ChunkData == ReadSndFileFormat::kDummyBinaryData; }
				[[nodiscard]] std::string_view Signature() const noexcept { return (IsDummy() ? ReadSndFileFormat::kDummyStringView : std::string_view(reinterpret_cast<const char*>(ChunkData), WAVEFormat::kdataSignature.size())); }
				[[nodiscard]] uint32_t ChunkSize() const noexcept { return (IsDummy() ? 0 : DecodeEndian::UInt32LE(&ChunkData[4])); }
				[[nodiscard]] const unsigned char* Comment() const noexcept { return (IsDummy() ? ReadSndFileFormat::kDummyBinaryData : &ChunkData[8]); }

				T_WAVESAECChunk(const unsigned char* const ChunkData) : ChunkData(ChunkData) {}
			};

			[[nodiscard]] bool CheckWAVEFormat() const noexcept {
				if (RIFFChunk().Signature() != WAVEFormat::kRIFFSignature) {
					T_ErrorHandle::Instance().SetError(ErrorMessage::Invalid_RIFFSignature);
					return false;
				}
				if (RIFFChunk().FormatSignature() != WAVEFormat::kFormatSignature) {
					T_ErrorHandle::Instance().SetError(ErrorMessage::Invalid_WAVEFormat);
					return false;
				}
				return true;
			}

		public:
			[[nodiscard]] ksize_t SoundStart() const noexcept { return kSoundStart; }
			[[nodiscard]] ksize_t SoundSize() const noexcept { return kSoundSize; }
			[[nodiscard]] T_WAVERIFFChunk RIFFChunk() const noexcept { return T_WAVERIFFChunk(&WAVEBinary()[kRIFFChunkOffset]); }
			[[nodiscard]] T_WAVEfmtChunk fmtChunk() const noexcept { return T_WAVEfmtChunk(&WAVEBinary()[fmtChunkOffset]); }
			[[nodiscard]] T_WAVEfactChunk factChunk() const noexcept { return T_WAVEfactChunk(&WAVEBinary()[factChunkOffset]); }
			[[nodiscard]] T_WAVEdataChunk dataChunk() const noexcept { return T_WAVEdataChunk(&WAVEBinary()[dataChunkOffset]); }
			[[nodiscard]] T_WAVESAECChunk SAECChunk() const noexcept { return T_WAVESAECChunk(!SAECChunkOffset ? ReadSndFileFormat::kDummyBinaryData : &WAVEBinary()[SAECChunkOffset]); }

			T_ReadWAVEBinary(const std::vector<unsigned char>& WAVEVector, ksize_t SoundSize) : kWAVEVector(WAVEVector), kSoundStart(static_cast<ksize_t>(WAVEVector.size() - SoundSize)), kSoundSize(SoundSize) {
				ReadWAVEBinary();
			}

			void ReadWAVEBinary() {
				if (!CheckWAVEFormat()) { return; }

				int32_t BinaryOffset = kRIFFChunkDataSize;
				auto TargetChunk = [this, &BinaryOffset]() -> const unsigned char* const { return &WAVEBinary()[BinaryOffset]; };
				auto TargetSignature = [&TargetChunk]() -> std::string_view { return std::string_view(reinterpret_cast<const char*>(TargetChunk()), 4); };
				auto TargetChunkSize = [&TargetChunk]() -> uint32_t { return DecodeEndian::UInt32LE(&TargetChunk()[4]); };
				auto NextTargetChunk = [&TargetChunkSize]() -> uint32_t { return TargetChunkSize() + 8 + (TargetChunkSize() & 1); };

				while (static_cast<ksize_t>(BinaryOffset) < kSoundSize && TargetChunkSize() >= 0) {
					if (TargetSignature() == WAVEFormat::kfmtSignature) {

						fmtChunkOffset = BinaryOffset;
					}
					else if (TargetSignature() == WAVEFormat::kfactSignature) {

						factChunkOffset = BinaryOffset;
					}
					else if (TargetSignature() == WAVEFormat::kdataSignature) {

						dataChunkOffset = BinaryOffset;
					}
					else if (TargetSignature() == WAVEFormat::kSAECSignature) {

						SAECChunkOffset = BinaryOffset;
					}
					BinaryOffset += NextTargetChunk();
				}
			}
		};

		struct T_SNDBinaryData {
		private:
			struct T_SoundList {
			private:
				const T_ReadWAVEBinary kWAVEBinary;
			public:
				[[nodiscard]] ksize_t SoundStart() const noexcept { return kWAVEBinary.SoundStart(); }
				[[nodiscard]] ksize_t SoundSize() const noexcept { return kWAVEBinary.SoundSize(); }
				[[nodiscard]] const T_ReadWAVEBinary& WAVEBinary() const noexcept { return kWAVEBinary; }

				T_SoundList(const std::vector<unsigned char>& WAVEVector, ksize_t SoundSize)
					: kWAVEBinary(WAVEVector, SoundSize) {
				}
			};

			struct T_DataList {
			private:
				const ksize_t kSoundListIndex;	// 配列Index
				const int64_t kSoundNumber;		// GroupNo(-2147483648～2147483647) ItemNo(-2147483648～2147483647)
			public:
				[[nodiscard]] ksize_t SoundListIndex() const noexcept { return kSoundListIndex; }
				[[nodiscard]] int32_t GroupNo() const noexcept { return Convert::DecodeSoundGroupNo(kSoundNumber); }
				[[nodiscard]] int32_t ItemNo() const noexcept { return Convert::DecodeSoundItemNo(kSoundNumber); }

				T_DataList(ksize_t SoundListIndex, int32_t GroupNo, int32_t ItemNo) noexcept
					: kSoundListIndex(SoundListIndex)
					, kSoundNumber(Convert::EncodeSoundNumber(GroupNo, ItemNo)) {
				}
			};

			std::vector<T_SoundList> SoundList_ = {};
			std::vector<T_DataList> DataList_ = {};
			std::vector<unsigned char> SoundBinary_ = {};

		public:
			[[nodiscard]] const std::vector<T_SoundList>& SoundList() const noexcept { return SoundList_; }
			[[nodiscard]] const std::vector<T_DataList>& DataList() const noexcept { return DataList_; }
			[[nodiscard]] const std::vector<unsigned char>& SoundBinary() const noexcept { return SoundBinary_; }
			[[nodiscard]] const T_SoundList& SoundList(ksize_t index) const noexcept { return SoundList_[index]; }
			[[nodiscard]] const T_DataList& DataList(ksize_t index) const noexcept { return DataList_[index]; }
			[[nodiscard]] ksize_t NumSound() const noexcept { return static_cast<ksize_t>(SoundList_.size()); }
			
			[[nodiscard]] const unsigned char* const Sound(ksize_t index) const noexcept {
				return SoundBinary_.data() + SoundList_[index].SoundStart();
			}

			[[nodiscard]] ksize_t SoundSize(ksize_t index) const noexcept {
				return SoundList_[index].SoundSize();
			}

			void AddDataList(ksize_t SoundListIndex, int32_t GroupNo, int32_t ItemNo) {
				DataList_.emplace_back(T_DataList(SoundListIndex, GroupNo, ItemNo));
			}

			void AddSound(const std::vector<unsigned char>& LoadSoundData) {
				SoundBinary_.insert(SoundBinary_.end(), LoadSoundData.begin(), LoadSoundData.end());
				SoundList_.emplace_back(T_SoundList(SoundBinary_, static_cast<ksize_t>(LoadSoundData.size())));
			}

		public:
			T_SNDBinaryData() = default;

			void reserve(ksize_t NumSound, ksize_t FileSize) {
				SoundList_.reserve(NumSound);
				DataList_.reserve(NumSound);
				SoundBinary_.reserve(FileSize);
			}

			void clear() {
				SoundList_.clear();
				DataList_.clear();
				SoundBinary_.clear();
			}

			void shrink_to_fit() {
				SoundList_.shrink_to_fit();
				DataList_.shrink_to_fit();
			}

			[[nodiscard]] bool empty() const noexcept {
				return SoundList_.empty() && DataList_.empty() && SoundBinary_.empty();
			}

			[[nodiscard]] ksize_t size() const noexcept {
				return static_cast<ksize_t>(SoundBinary_.size());
			}
		};

		struct T_LoadSNDHeader {
		private:
			const std::string kFileName = {};
			const std::string kFilePath = {};
			const uintmax_t kFileSize = 0;
			std::ifstream File = {};
			unsigned char buffer[24] = {};
			const bool kCheckError = false;
			// 0～11 識別子("ElecbyteSnd")
			// 12～ メイン情報
			// 24～ コメント

			[[nodiscard]] const std::string EnsureSndExtension(const std::filesystem::path& FileName) const {
				std::filesystem::path FixedFileName = FileName;
				if (FixedFileName.extension() != SNDFormat::kExtension) {
					if (!FixedFileName.extension().empty()) {
						T_ErrorHandle::Instance().SetError(ErrorMessage::Invalid_SNDExtension);
					}
					FixedFileName.replace_extension(SNDFormat::kExtension);
				}
				return FixedFileName.string();
			}

			[[nodiscard]] const std::string FindFilePathDown(const std::string& FilePath) const {
				T_FilePathSystem SNDFolder;
				if (!FilePath.empty()) {
					SNDFolder.SetPath(FilePath);
					if (SNDFolder.ErrorCode()) {
						T_ErrorHandle::Instance().SetError(ErrorMessage::Invalid_LoadSNDPath);
					}
				}
				if (FilePath.empty() || SNDFolder.ErrorCode() && !T_Config::Instance().SNDSearchPath().empty()) {
					SNDFolder.SetPath(T_Config::Instance().SNDSearchPath());
					if (SNDFolder.ErrorCode()) {
						T_ErrorHandle::Instance().SetError(ErrorMessage::Invalid_SNDSearchPath);
					}
				}
				const std::filesystem::path AbsolutePath = (std::filesystem::exists(SNDFolder.Path()) ? SNDFolder.Path() : std::filesystem::canonical(std::filesystem::current_path()));

				for (const auto& entry : std::filesystem::recursive_directory_iterator(
					AbsolutePath, std::filesystem::directory_options::skip_permission_denied)) {
					if (!entry.is_regular_file()) { continue; }
					if (entry.path().filename() == kFileName) {
						return entry.path().string();
					}
				}

				T_ErrorHandle::Instance().SetError(ErrorMessage::NotFound_SNDFile);
				return {};
			}

			[[nodiscard]] bool CheckFileSize() const {
				if (kFileSize <= UINT32_MAX) { return false; }
				T_ErrorHandle::Instance().SetError(ErrorMessage::Invalid_SNDFileSize);
				return true;
			}
			[[nodiscard]] bool CheckFilePath() const {
				if (!FilePath().empty()) { return false; }
				T_ErrorHandle::Instance().SetError(ErrorMessage::Invalid_EmptySNDFilePath);
				return true;
			}
			[[nodiscard]] bool CheckFileOpen() {
				File.open(FilePath(), std::ios::binary);
				if (File.is_open()) { return false; }
				T_ErrorHandle::Instance().SetError(ErrorMessage::Failed_OpenSNDFile);
				return true;
			}

			[[nodiscard]] bool CheckSNDFormat() {
				// ファイル読み取りが絡むので一か所にまとめた
				File.read(reinterpret_cast<char*>(&buffer), sizeof(buffer));

				if (Signature() != SNDFormat::kSignature) {
					T_ErrorHandle::Instance().SetError(ErrorMessage::Invalid_SNDSignature);
					return true;
				}
				
				if (SubHeaderStart() != SNDFormat::kSubHeaderStart) {
					T_ErrorHandle::Instance().SetError(ErrorMessage::Corrupted_SNDFile);
					return true;
				}
				return false;
			}

			[[nodiscard]] bool CheckFileError() { // CheckFileOpen → CheckSNDFormat の順で実行
				return CheckFileSize() || CheckFilePath() || CheckFileOpen() || CheckSNDFormat();
			}

		public:
			[[nodiscard]] const std::string& FileName() const noexcept { return kFileName; }
			[[nodiscard]] const std::string& FilePath() const noexcept { return kFilePath; }
			[[nodiscard]] ksize_t FileSize() const noexcept { return static_cast<ksize_t>(kFileSize); }
			[[nodiscard]] std::string_view Signature() const noexcept { return std::string_view(reinterpret_cast<const char*>(buffer), SNDFormat::kSignature.size()); }
			[[nodiscard]] uint32_t Version() const noexcept { return DecodeEndian::UInt32BE(&buffer[12]); }
			[[nodiscard]] uint32_t NumSound() const noexcept { return DecodeEndian::UInt32LE(&buffer[16]); }
			[[nodiscard]] uint32_t SubHeaderStart() const noexcept { return DecodeEndian::UInt32LE(&buffer[20]); }
			[[nodiscard]] bool CheckError() const noexcept { return kCheckError; }

		public:
			T_LoadSNDHeader(const std::string& FileName, const std::string& FilePath)
				: kFileName(EnsureSndExtension(FileName)), kFilePath(FindFilePathDown(FilePath))
				, kFileSize(kFilePath.empty() ? 0 : std::filesystem::file_size(kFilePath)), kCheckError(CheckFileError()) {
			}

			void seekg(std::streampos& _Pos, std::ios_base::seekdir _Way = std::ios::beg) {
				File.seekg(_Pos, _Way);
			}
			void seekg(uint32_t _Pos, std::ios_base::seekdir _Way = std::ios::beg) {
				File.seekg(_Pos, _Way);
			}

			void read(char* _Str, std::streamsize _Count) {
				File.read(_Str, _Count);
			}

			[[nodiscard]] std::streampos tellg() {
				return File.tellg();
			}
		};

		// SND読み込み時のサブヘッダー情報格納先
		struct T_LoadSNDSubHeader {
		private:
			unsigned char buffer[16] = {}; // 主要部分の情報のみ格納
			// 0～16 メイン情報
			// 17～  WAVEデータ
			T_LoadSNDHeader& File;
			std::vector<unsigned char> WAVEBinary = {};

			void InitLoadSNDSubHeader() {
				File.seekg(File.SubHeaderStart());
			}

			[[nodiscard]] bool CheckReadError() { // データの末尾に到達orアドレスが不正な値
				return !NextAddress() || NextAddress() < File.tellg();
			}

			[[nodiscard]] bool ReadSubHeader() {
				File.read(reinterpret_cast<char*>(&buffer), sizeof(buffer));
				return CheckReadError();
			}

		public:
			[[nodiscard]] uint32_t NextAddress() const noexcept { return DecodeEndian::UInt32LE(&buffer[0]); }
			[[nodiscard]] uint32_t DataSize() const noexcept { return DecodeEndian::UInt32LE(&buffer[4]); }
			[[nodiscard]] uint32_t GroupNo() const noexcept { return DecodeEndian::UInt32LE(&buffer[8]); }
			[[nodiscard]] uint32_t ItemNo() const noexcept { return DecodeEndian::UInt32LE(&buffer[12]); }
			[[nodiscard]] bool CheckError() { return false; } // 処理が思いつかないので保留

		public:
			T_LoadSNDSubHeader(T_LoadSNDHeader& LoadSNDHeader) : File(LoadSNDHeader) {
				InitLoadSNDSubHeader();
			}

			[[nodiscard]] bool ReadSoundBinary(T_UnorderedMap<int64_t>& SoundNumberUMap, T_UnorderedMap<int32_t>& SoundGroupNoUMap, T_SNDBinaryData& SNDBinaryData) {
				if (ReadSubHeader()) { return true; }

				// 取得したサウンド番号が重複
				if (SoundNumberUMap.exist(GroupNo(), ItemNo())) {
					T_ErrorHandle::Instance().SetError(ErrorMessage::Warning_DuplicateSoundNumber, GroupNo(), ItemNo());
					File.seekg(NextAddress());
					return false;
				}

				SoundNumberUMap.Register(GroupNo(), ItemNo());

				// グループ数の取得用
				if (!SoundGroupNoUMap.exist(GroupNo())) {
					SoundGroupNoUMap.Register(GroupNo());
				}

				// サウンドデータ一時保存
				WAVEBinary.resize(DataSize());
				File.read(reinterpret_cast<char*>(WAVEBinary.data()), WAVEBinary.size());

				ksize_t SoundListIndex = 0;
				bool FoundSoundData = false;

				// サウンドデータ重複チェック
				for (SoundListIndex = 0; SoundListIndex < SNDBinaryData.NumSound(); ++SoundListIndex) {
					// 既存のサウンドデータの場合はインデックスを指定
					if (SNDBinaryData.SoundSize(SoundListIndex) == WAVEBinary.size() && !std::memcmp(SNDBinaryData.Sound(SoundListIndex), WAVEBinary.data(), WAVEBinary.size())) {
						FoundSoundData = true;
						break;
					}

				}
				// 新規サウンドの場合はサウンドデータを格納
				if (!FoundSoundData) {
					SoundListIndex = static_cast<ksize_t>(SNDBinaryData.SoundList().size());
					SNDBinaryData.AddSound(WAVEBinary);
				}

				SNDBinaryData.AddDataList(SoundListIndex, GroupNo(), ItemNo());

				File.seekg(NextAddress());
				
				return false;
			}
		};

		struct T_SNDData {
			int32_t NumGroup_ = 0;
			int32_t NumItem_ = 0;
			std::string FileName_ = {};
			T_UnorderedMap<int64_t> SoundNumberUMap = {};
			T_UnorderedMap<int32_t> SoundGroupNoUMap = {};
			T_SNDBinaryData SNDBinaryData = {};

			void NumGroup(int32_t value) noexcept { NumGroup_ = value; }
			void NumItem(int32_t value) noexcept { NumItem_ = value; }
			void FileName(const std::string& value) noexcept { FileName_ = value; }

			void ReserveData(T_LoadSNDHeader& LoadSNDHeader) {
				const ksize_t kNumSound = LoadSNDHeader.NumSound();
				const ksize_t kFileSize = LoadSNDHeader.FileSize();

				SoundNumberUMap.reserve(kNumSound);
				SoundGroupNoUMap.reserve(kNumSound);
				SNDBinaryData.reserve(kNumSound, kFileSize);
			}

			void shrink_to_fit() {
				SoundNumberUMap.shrink_to_fit();
				SoundGroupNoUMap.shrink_to_fit();
				SNDBinaryData.shrink_to_fit();
			}

			bool LoadSNDFile(const std::string& FileName_, const std::string& FilePath_) {
				if (!empty()) { clear(); }
				T_LoadSNDHeader LoadSNDHeader(FileName_, FilePath_);
				if (LoadSNDHeader.CheckError()) { return false; }
				T_LoadSNDSubHeader LoadSNDSubHeader(LoadSNDHeader);
				if (LoadSNDSubHeader.CheckError()) { return false; }

				NumItem(LoadSNDHeader.NumSound());
				FileName(LoadSNDHeader.FileName());
				ReserveData(LoadSNDHeader);
				
				for (int32_t LoadNo = 0; LoadNo < NumItem(); ++LoadNo) {
					if (LoadSNDSubHeader.ReadSoundBinary(SoundNumberUMap, SoundGroupNoUMap, SNDBinaryData)) { break; };
				}
				NumGroup(static_cast<int32_t>(SoundGroupNoUMap.size()));
			
				// 全てのロードが終了したら余分に確保したメモリを解放
				shrink_to_fit();

				// ログ出力
				if (T_Config::Instance().CreateLogFile()) {
					T_FilePathSystem SAELibFile(T_Config::Instance().SAELibFilePath() / (T_Config::Instance().CreateSAELibFile() ? ReadSndFileFormat::kSystemDirectoryName : ""));
					if (SAELibFile.ErrorCode()) {
						T_ErrorHandle::Instance().SetError(ErrorMessage::Invalid_SAELibFolderPath);
						return false;
					}
					if (T_Config::Instance().CreateSAELibFile()) {
						SAELibFile.CreateDirectory(SAELibFile.Path());
						if (SAELibFile.ErrorCode()) {
							T_ErrorHandle::Instance().SetError(ErrorMessage::Failed_CreateSAELibFolder);
							return false;
						}
					}

					const std::string ErrorLogFileName = std::string(ReadSndFileFormat::kErrorLogFileName) + "_" + FileName() + ".txt";
					std::ofstream ErrorLogFile(SAELibFile.Path() / ErrorLogFileName);
					if (!ErrorLogFile.is_open()) {
						T_ErrorHandle::Instance().SetError(ErrorMessage::Failed_CreateErrorLogFile);
					}
					T_ErrorHandle::Instance().WriteErrorLog(ErrorLogFile);
				}

				return true;
			}

			// ユーザー向けのT_DataListアクセス手段
			struct T_AccessData {
			private:
				const T_SNDBinaryData* const kSNDBinaryDataPtr;
				const ksize_t kDataListIndex; // 配列Index(最大値のときダミーデータフラグとして使用)

				const auto& DataListRef() const noexcept { return kSNDBinaryDataPtr->DataList(kDataListIndex); }
				const auto& WAVEBinaryRef() const noexcept { return kSNDBinaryDataPtr->SoundList(DataListRef().SoundListIndex()).WAVEBinary(); }

				// SAEで表示されるフレーム秒数の算出用定数
				inline static constexpr double kFramesPerSecond = 60.0;

			public:
				/**
				* @brief ダミーデータ判断
				*
				* 　自身がダミーデータであるかを確認します
				*
				* 　SNDConfig::SetThrowErrorの設定がOFFの場合にエラー回避のために使用されます
				*
				* @return bool (false = 自身が正常なデータ：true = 自身がダミーデータ)
				*/
				bool IsDummy() const noexcept { return kDataListIndex == KSIZE_MAX; }

				/**
				* @brief グループ番号の取得
				*
				* 　対象音声のグループ番号を返します
				*
				* 　ダミーデータの場合は 0 を返します
				*
				* @return int32_t GroupNo グループ番号
				*/
				int32_t GroupNo() const noexcept { return (IsDummy() ? 0 : DataListRef().GroupNo()); }

				/**
				* @brief アイテム番号の取得
				*
				* 　対象音声のアイテム番号を返します
				*
				* 　ダミーデータの場合は 0 を返します
				*
				* @return int32_t ImageNo アイテム番号
				*/
				int32_t ItemNo() const noexcept { return (IsDummy() ? 0 : DataListRef().ItemNo()); }

				/**
				* @brief バイトサイズの取得
				*
				* 　対象音声のバイトサイズを返します
				*
				* 　ダミーデータの場合は 0 を返します
				*
				* @return int32_t ByteSize バイトサイズ
				*/
				int32_t ByteSize() const noexcept { return (IsDummy() ? 0 : static_cast<int32_t>(WAVEBinaryRef().dataChunk().ChunkSize())); }

				/**
				* @brief チャンネルの取得
				*
				* 　対象音声のチャンネル数を返します
				*
				* 　ダミーデータの場合は 0 を返します
				*
				* @return int32_t Channel チャンネル
				*/
				int32_t Channel() const noexcept { return (IsDummy() ? 0 : static_cast<int32_t>(WAVEBinaryRef().fmtChunk().Channels())); }

				/**
				* @brief ヘルツの取得
				*
				* 　対象音声のヘルツを返します
				*
				* 　ダミーデータの場合は 0 を返します
				*
				* @return int32_t Hz ヘルツ
				*/
				int32_t Hz() const noexcept { return (IsDummy() ? 0 : static_cast<int32_t>(WAVEBinaryRef().fmtChunk().SamplesPerSec())); }

				/**
				* @brief ビットの取得
				*
				* 　対象音声のビットを返します
				*
				* 　ダミーデータの場合は 0 を返します
				*
				* @return int32_t Bit ビット
				*/
				int32_t Bit() const noexcept { return (IsDummy() ? 0 : static_cast<int32_t>(WAVEBinaryRef().fmtChunk().BitsPerSample())); }

				/**
				* @brief サンプル秒数の取得
				*
				* 　対象音声のサンプル秒数を返します
				*
				* 　ダミーデータの場合は 0 を返します
				*
				* @return double SampleRate サンプル秒数
				*/
				double SampleRate() const noexcept { return (IsDummy() ? 0 : double(ByteSize()) / double(WAVEBinaryRef().fmtChunk().AvgBytesPerSec())); }
				
				/**
				* @brief フレーム秒数の取得
				*
				* 　対象音声のフレーム秒数を返します
				*
				* 　ダミーデータの場合は 0 を返します
				*
				* @return double SampleFrame フレーム秒数
				*/
				double SampleFrame() const noexcept { return (IsDummy() ? 0 : SampleRate() * kFramesPerSecond); }

				/**
				* @brief コメントの取得
				*
				* 　SAEで設定された対象音声のコメントを返します
				*
				* 　ダミーデータの場合は DummyBinaryData を返します
				* 
				* 　DummyBinaryData は常に長さ1の配列で内容は {0} です
				*
				* @return const unsigned char* const Comment コメントデータ配列
				* @retval const unsigned char* const DummyBinaryData ダミーデータ配列
				*/
				const unsigned char* Comment() const noexcept { return (IsDummy() ? ReadSndFileFormat::kDummyBinaryData : WAVEBinaryRef().SAECChunk().Comment()); }

				T_AccessData(const T_SNDBinaryData* const SNDBinaryDataPtr, const ksize_t DataListIndex) : kSNDBinaryDataPtr(SNDBinaryDataPtr), kDataListIndex(DataListIndex) {}
			};

		public:
			/**
			* @brief SNDデータのサウンドグループ数を取得
			*
			* 　読み込んだSNDデータのサウンドグループ数を返します
			*
			* @return int32_t NumGroup サウンドグループ数
			*/
			int32_t NumGroup() const noexcept { return NumGroup_; }

			/**
			* @brief SNDデータのサウンド数を取得
			*
			* 　読み込んだSNDデータのサウンド数を返します
			*
			* @return int32_t NumItem サウンド数
			*/
			int32_t NumItem() const noexcept { return NumItem_; }

			/**
			* @brief SNDデータのファイル名を取得
			*
			* 　読み込んだSNDデータの拡張子を除いたファイル名を返します
			*
			* @return const std::string& FileName ファイル名
			*/
			const std::string& FileName() const noexcept { return FileName_; }

			/**
			* @brief SNDデータの初期化
			*
			* 　読み込んだSNDデータを初期化します
			*
			* @note
			*/
			void clear() {
				NumGroup(0);
				NumItem(0);
				FileName_.clear();
				SoundNumberUMap.clear();
				SoundGroupNoUMap.clear();
				SNDBinaryData.clear();
			}

			/**
			* @brief SNDデータの存在確認
			*
			* 　読み込んだSNDデータの空かを判定します
			*
			* @return bool 判定結果 (false = データが存在：true = データが空)
			*/
			bool empty() const noexcept {
				return FileName().empty() && SNDBinaryData.empty() && SoundNumberUMap.empty() && SoundGroupNoUMap.empty();
			}

			/**
			* @brief SNDデータのデータサイズを取得
			*
			* 　読み込んだSNDデータのデータサイズを返します
			*
			* @return size_t SNDDataSize SNDデータサイズ
			*/
			size_t size() const noexcept {
				return SNDBinaryData.size();
			}

		public:
			using SoundData = T_AccessData;

			T_SNDData() = default;

			T_SNDData(const std::string& FileName, const std::string& FilePath = "")
			{
				LoadSNDFile(FileName, FilePath);
			}

			/**
			* @brief 指定されたSNDファイルを読み込み
			*
			* 　実行ファイルから子階層へファイル名を検索して読み込みます
			*
			* 　第二引数指定時は指定した階層からファイル名を検索します(SNDConfigよりも優先されます)
			*
			* 　実行時に既存の要素は初期化、上書きされます
			*
			* @param const std::string& FileName ファイル名 (拡張子 .snd は省略可)
			* @param const std::string& FilePath 対象のパス (省略時は実行ファイルの子階層を探索)
			* @return bool 読み込み結果 (false = 失敗：true = 成功)
			*/
			bool LoadSND(const std::string& FileName, const std::string& FilePath = "") {
				return LoadSNDFile(FileName, FilePath);
			}

			/**
			* @brief 指定番号の存在確認
			*
			* 　読み込んだSNDデータを検索し、指定番号が存在するかを確認します
			*
			* @param int32_t GroupNo グループ番号
			* @param int32_t ItemNo アイテム番号
			* @return bool 検索結果 (false = 存在なし : true = 存在あり)
			*/
			bool ExistSoundNumber(int32_t GroupNo, int32_t ItemNo) {
				return SoundNumberUMap.exist(GroupNo, ItemNo);
			}

			/**
			* @brief 指定番号のデータへアクセス
			*
			* 　指定したグループ番号とアイテム番号のSNDデータへアクセスします
			*
			* 　対象が存在しない場合はSNDConfig::SetThrowErrorの設定に準拠します
			*
			* @param int32_t GroupNo グループ番号
			* @param int32_t ItemNo アイテム番号
			* @retval 対象が存在する SoundData
			* @retval 対象が存在しない SNDConfig::SetThrowError (false = ダミーデータの参照：true = 例外を投げる)
			*/
			const SoundData GetSoundData(int32_t GroupNo, int32_t ItemNo) {
				if (int32_t SoundNumber = SoundNumberUMap.find(GroupNo, ItemNo); SoundNumber >= 0) { // SoundExist(GroupNo, SoundNo)と同義
					return SoundData(&SNDBinaryData, SoundNumber);
				}
				if (!T_Config::Instance().ThrowError()) {
					return SoundData(&SNDBinaryData, KSIZE_MAX);
				}
				T_ErrorHandle::Instance().ThrowError(ErrorMessage::NotFound_SoundNumber, GroupNo, ItemNo);
			}

			/**
			* @brief 指定インデックスデータの存在確認
			*
			* 　読み込んだSNDデータを検索し、指定インデックスのデータ存在するかを確認します
			*
			* @param int32_t index データ配列インデックス
			* @return bool 検索結果 (false = 存在なし : true = 存在あり)
			*/
			bool ExistSoundDataIndex(int32_t SoundDataIndex) const {
				return static_cast<ksize_t>(SoundDataIndex) < SoundNumberUMap.size();
			}

			/**
			* @brief 指定インデックスのデータへアクセス
			*
			* 　SNDデータへ指定したインデックスでアクセスします
			*
			* 　対象が存在しない場合はSNDConfig::SetThrowErrorの設定に準拠します
			*
			* @param int32_t index データ配列インデックス
			* @retval 対象が存在する SoundData
			* @retval 対象が存在しない SNDConfig::SetThrowError (false = ダミーデータの参照：true = 例外を投げる)
			*/
			const SoundData GetSoundDataIndex(int32_t index) const {
				if (ExistSoundDataIndex(index)) {
					return SoundData(&SNDBinaryData, index);
				}
				if (!T_Config::Instance().ThrowError()) {
					return SoundData(&SNDBinaryData, KSIZE_MAX);
				}
				T_ErrorHandle::Instance().ThrowError(ErrorMessage::NotFound_SoundIndex, index);
			}

		}; // struct T_SNDData
	} // ReadSndFile_detail

	// 使用ユーザー向けの名前設定

	/**
	* @brief SNDファイルを扱うクラス
	*
	* 　- コンストラクタの引数を指定した場合、指定した引数でLoadSND関数を実行します
	*
	* 　- 引数を指定しない場合、ファイル読み込みは行いません
	*
	* @param const std::string& FileName ファイル名 (拡張子 .snd は省略可)
	* @param const std::string& FilePath 対象のパス (省略時は実行ファイルの子階層を探索)
	*/
	using SND = ReadSndFile_detail::T_SNDData;

	/**
	* @brief ReadSndFileのエラー情報空間
	*/
	namespace SNDError = ReadSndFile_detail::ErrorMessage;

	/**
	* @brief ReadSndFileのコンフィグ設定空間
	*/
	namespace SNDConfig {

		///////////////////////////////////////////////////////////////////////////////////////////////////
		// Setter /////////////////////////////////////////////////////////////////////////////////////////
		///////////////////////////////////////////////////////////////////////////////////////////////////
		/**
		* @brief エラー出力の切り替え設定
		*
		* 　このライブラリ関数で発生したエラーを例外として投げるかログとして記録するかを指定できます
		*
		* @param bool flag (false = ログとして記録する：true = 例外を投げる)
		*/
		inline void SetThrowError(bool flag) { ReadSndFile_detail::T_Config::Instance().ThrowError(flag); }

		/**
		* @brief エラーログファイルを作成設定
		*
		* 　このライブラリ関数で発生したエラーのログファイルを出力するかどうか指定できます
		*
		* @param bool flag (false = ログファイルを出力しない：true = ログファイルを出力する)
		*/
		inline void SetCreateLogFile(bool flag) { ReadSndFile_detail::T_Config::Instance().CreateLogFile(flag); }

		/**
		* @brief SAELibフォルダを作成設定
		*
		* 　ファイルの出力先としてSAELibファイルを使用するかを指定できます
		*
		* @param bool flag (false = SAELibファイルを使用しない：true = SAELibファイルを使用する)
		* @param const std::string& Path SAELibフォルダ作成先 (省略時はパスの設定なし)
		*/
		inline void SetCreateSAELibFile(bool flag, const std::string& Path = "") {
			ReadSndFile_detail::T_Config::Instance().CreateSAELibFile(flag);
			if (!Path.empty()) {
				ReadSndFile_detail::T_Config::Instance().SAELibFilePath(Path);
			}
		}

		/**
		* @brief SAELibフォルダのパス設定
		*
		* 　SAELibファイルの作成パスを指定できます
		*
		* @param const std::string& Path SAELibフォルダ作成先
		*/
		inline void SetSAELibFilePath(const std::string& Path = "") { ReadSndFile_detail::T_Config::Instance().SAELibFilePath(Path); }

		/**
		* @brief SNDファイルの検索パス設定
		*
		* 　SNDファイルの検索先のパスを指定できます
		*
		* 　SNDコンストラクタもしくはLoadSND関数で検索先のパスを指定しない場合、この設定のパスで検索します
		*
		* @param const std::string& Path SNDファイルの検索先のパス
		*/
		inline void SetSNDSearchPath(const std::string& Path = "") { ReadSndFile_detail::T_Config::Instance().SNDSearchPath(Path); }

		///////////////////////////////////////////////////////////////////////////////////////////////////
		// Getter /////////////////////////////////////////////////////////////////////////////////////////
		///////////////////////////////////////////////////////////////////////////////////////////////////
		/**
		* @brief エラー出力切り替え設定取得
		*
		* 　Config設定のエラー出力切り替え設定を取得します
		*
		* @return bool エラー設定出力切り替え設定(false = OFF：true = ON)
		*/
		inline bool GetThrowError() { return ReadSndFile_detail::T_Config::Instance().ThrowError(); }

		/**
		* @brief エラーログファイルを作成設定取得
		*
		* 　Config設定のエラーログファイルを作成設定を取得します
		*
		* @return bool エラーログファイルを作成設定(false = OFF：true = ON)
		*/
		inline bool GetCreateLogFile() { return ReadSndFile_detail::T_Config::Instance().CreateLogFile(); }

		/**
		* @brief SAELibフォルダを作成設定取得
		*
		* 　Config設定のSAELibフォルダを作成設定を取得します
		*
		* @return bool SAELibフォルダを作成設定(false = OFF：true = ON)
		*/
		inline bool GetCreateSAELibFile() { return ReadSndFile_detail::T_Config::Instance().CreateSAELibFile(); }

		/**
		* @brief Config設定取得
		*
		* 　Config設定のフラグをまとめて取得します
		*
		* @return int32_t Config設定
		*/
		inline int32_t GetConfigFlag() { return ReadSndFile_detail::T_Config::Instance().BitFlag(); }

		/**
		* @brief SAELibフォルダを作成パス取得
		*
		* 　Config設定のSAELibフォルダを作成パスを取得します
		*
		* @return const std::filesystem::path& SAELibフォルダを作成パス
		*/
		inline const std::filesystem::path& GetSAELibFilePath() { return ReadSndFile_detail::T_Config::Instance().SAELibFilePath(); }

		/**
		* @brief SNDファイルの検索パス取得
		*
		* 　Config設定のSNDファイルの検索パスを取得します
		*
		* @return const std::filesystem::path& SNDファイルの検索パス
		*/
		inline const std::filesystem::path& GetSNDSearchPath() { return ReadSndFile_detail::T_Config::Instance().SNDSearchPath(); }
	}
} // namespace SAELib
#endif