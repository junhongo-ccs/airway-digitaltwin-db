# spaceInfraCpp __
## 環境インスト―ル手順
### ルートフォルダの作成
```
mkdir c:/cppProjectRoot
```
### リポジトリルートの作成
```
cd c:/cppProjectRoot
mkdir project
```
### クローン
上記フォルダをルートに　spaceInfra-cppを配置する 

今後　c:/cppProjectRoot/project/spaceInfraCpp　をリポジトリルートと呼ぶ 

### Visual Studio2022 をインストールする

C++によるデスクトップ開発　を選択してインストールします


### vc のC++バージョンを　C++17　に設定する

#### 追加インストール時

cd c:\src\vcpkg

当project　で必要なライブラリ 

【MakeCityGmlLas】
以下の手順に従ってlibcitygmlをインストールする
・プロジェクトに含まれているフォルダlibcitygmlをvkpkgのフォルダportsの下にコピーする
・以下のコマンドを実行する
  .\vcpkg\vcpkg install libcitygml:x64-windows

#### Visual Studioへのライブラリマネージャ(vcpkg)の統合

cd c:\src\vcpkg

\vcpkg\vcpkg integrate install   　

### MySQLのDB作成
データベースspatial_db を作成し、テーブル作成

CREATE TABLE `space_data_manages` (
    `id` BIGINT UNSIGNED AUTO_INCREMENT PRIMARY KEY,
    `owner_id` VARCHAR(32),
    `batch_id` VARCHAR(32),
    `spatial_id` VARCHAR(32),
    `file_id` VARCHAR(32),
    `file_path` VARCHAR(256),
    `epsg_las` VARCHAR(16),
    `epsg_csv` VARCHAR(16),
    `data_unit` VARCHAR(16),
    `data_io` VARCHAR(16),
    `data_type` VARCHAR(16),
    `data_status` VARCHAR(16),
    `data_abcd` VARCHAR(16),
    `sp_xmin` DOUBLE,
    `sp_xmax` DOUBLE,
    `sp_ymin` DOUBLE,
    `sp_ymax` DOUBLE,
    `sp_zmin` DOUBLE,
    `sp_zmax` DOUBLE,
    `originalFileIds` VARCHAR(256),
    `points` INT,
    `xmin` DOUBLE,
    `xmax` DOUBLE,
    `ymin` DOUBLE,
    `ymax` DOUBLE,
    `zmin` DOUBLE,
    `zmax` DOUBLE,
    `cls0` INT,
    `cls1` INT,
    `cls2` INT,
    `cls3` INT,
    `cls4` INT,
    `cls5` INT,
    `cls6` INT,
    `cls7` INT,
    `cls8` INT,
    `cls9` INT,
    `cls10` INT,
    `cls11` INT,
    `cls12` INT,
    `cls13` INT,
    `cls14` INT,
    `cls15` INT,
    `cls16` INT,
    `cls17` INT,
    `cls18` INT,
    `cls19` INT,
    `cls20` INT,
    `cls99` INT,
    `created_at` TIMESTAMP NULL DEFAULT NULL,
    `updated_at` TIMESTAMP NULL DEFAULT NULL,
    `deleted_at` TIMESTAMP NULL DEFAULT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;


### MySQL Connector/C++ のインストール

https://dev.mysql.com/downloads/connector/cpp/

Select Operating System:Microsoft Windows

Select OS Version:Windows (x86, 64-bit)　　　を選択。

「ZIP Archive」をdownloadして解凍したフォルダの名前を
「MySQL Connector C++ 8.0」に変更(単語間のスペースも必要！)

C:\Program Files\MySQL　の下に配置する

結果、該当フォルダは
C:\Program Files\MySQL\MySQL Connector C++ 8.0
となる

 
### API用リポジトリ　spatialIdの取り込み
#### spatialIdの配置

c:/cppProjectRoot/project　に　spatialId　を配置する

#### 自分のソリューションにspatialIdのプロジェクトを追加する

visualStudio　で　取り込みたいアプリのプロジェクトを開く
ソリューションエクスプローラーの　ソリューション名を右クリック　→　追加　→　既存プロジェクト　→　spatialIdのvcxproj選択
ソリューションエクスプローラーに　spatialId　が　表示される

#### SpatialIdについてはLibを生成するように変更

上記の　spatialId　を右クリックし、プロパティページを開きます
構成の種類をスタティックライブラリに変更します

#### 自分のプロジェクトについてSpatialIdを参照するように設定

ソリューションエクスプローラーの　自分のアプリの　参照を右クリック　参照の　追加をクリック
ポップアップの中から　表示された　spatialId　のチェックボックスをチェックします

#### includeの指定方法

#include "../../SpatialId/src/spatial_api.h"


### vsコードでのアプリビルド
新規の場合、又は　アプリ修正を取り込む場合、実行環境の　VCでビルドを行います

１）VisualStudio2022を起動します

２）プロジェクトやソリューションを開くで下記を選択します

　　　C:\cppProjectRoot\project\spaceInfraCpp\プログラムID\プログラムID.sln"

３）ソリューションエクスプローラで　ソースファイル＞プログラムID.cpp　を選択

４）ヘッダーの選択リストで　　Release　　x64　を　選択

５）ビルド　＞　プログラムIDのビルド　を実行

