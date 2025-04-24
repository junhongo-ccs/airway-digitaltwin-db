# SPATIALID

空間API for C++版

## 開発環境

- Windows
     - Visual Studio 2022 (C/C++) x64

## C++バージョン

- C++17 以降
     - Windows
          - Visual Studio 2022 の [プロジェクト] → [プロパティ] → [構成プロパティ] → [全般] → [C++言語標準] で "ISO C++17 標準 (/std:c++17)" を指定


## オープンソースライブラリ

- [PROJ 9](https://proj.org/en/9.2/index.html)
     - 用途: 座標変換
     - [ライセンス: MIT](https://proj.org/en/9.2/about.html#license)
     - [インストール方法](https://proj.org/install.html)

          - Windows
               - osgeo4w-setup.exe をダウンロードし、実行する
               - インストール場所は C:¥OSGeo4W に行う
               - OSGeo4Wセットアップの"パッケージの選択"で必要なもの
                    ```
                    [Libs]
                    |- [libwebp]
                    |- [proj-devel]
                    |- [proj91-runtime]
                    ```

     - ビルド設定

          - Windows
               - インクルードディレクトリ
                    - Visual Studio の該当プロジェクトの [プロパティページ] → [構成：すべての構成] → [C/C++] → [全般] → [追加のインクルードディレクトリ] で下記のパス名を追加してください。(複数の場合 ; で区切ってください。)
                         ```
                         C:\OSGeo4W\include
                         ```
               - ライブラリディレクトリ
                    - Visual Studio の該当プロジェクトの [プロパティページ] → [構成：すべての構成] → [リンカー] → [全般] → [追加のライブラリディレクトリ] で下記のパス名を追加してください。(複数の場合 ; で区切ってください。)
                         ```
                         C:\OSGeo4W\lib
                         ```
               - ライブラリ
                    - Visual Studio の該当プロジェクトの [プロパティページ] → [構成：すべての構成] → [リンカー] → [入力] → [追加の依存ファイル] で下記のファイル名を追加してください。(複数の場合 ; で区切ってください。)
                         ```
                         proj.lib
                         ```
               - 動作に必要なモジュールは C:¥OSGeo4¥bin にある

- [Bullet Physics SDK](https://github.com/bulletphysics/bullet3)
     - 用途: 衝突判定
     - [ライセンス: zlib](https://github.com/bulletphysics/bullet3/blob/master/LICENSE.txt)
     - インストール方法（README.md参照）
          - Windows
               ```
               cd C:¥
               git clone https://github.com/Microsoft/vcpkg.git
               cd vcpkg
               bootstrap-vcpkg.bat
               vcpkg integrate install
               vcpkg install bullet3:x64-windows
               ```
     - ビルド設定

          - Windows
               - インクルードディレクトリ
                    - Visual Studio の該当プロジェクトの [プロパティページ] → [構成：すべての構成] → [C/C++] → [全般] → [追加のインクルードディレクトリ] で下記のパス名を追加してください。(複数の場合 ; で区切ってください。)
                         ```
                         C:\vcpkg\installed\x64-windows\include\bullet
                         ```
               - ライブラリディレクトリ
                    - Visual Studio の該当プロジェクトの [プロパティページ] → [構成：すべての構成] → [リンカー] → [全般] → [追加のライブラリディレクトリ] で下記のパスを追加してください。(複数の場合 ; で区切ってください。)
                         ```
                         C:\vcpkg\installed\x64-windows\lib
                         ```
               - ライブラリ: Debug版
                    - Visual Studio の該当プロジェクトの [プロパティページ] → [構成：Debug] → [リンカー] → [入力] → [追加の依存ファイル] で下記のファイル名を追加してください。(複数の場合 ; で区切ってください。)
                         ```
                         Bullet3Common_Debug.lib
                         BulletCollision_Debug.lib
                         BulletDynamics_Debug.lib
                         LinearMath_Debug.lib
                         ```
               - ライブラリ: Release版
                    - Visual Studio の該当プロジェクトの [プロパティページ] → [構成：Release] → [リンカー] → [入力] → [追加の依存ファイル] で下記のファイル名を追加してください。(複数の場合 ; で区切ってください。)
                         ```
                         Bullet3Common.lib
                         BulletCollision.lib
                         BulletDynamics.lib
                         LinearMath.lib
                         ```

- [SQLite3](https://www.sqlite.org/index.html)
     - 用途: 空間ボクセルの保存
     - [ライセンス: SQLite](https://www.sqlite.org/copyright.html)
     - インストール方法

          - Windows

               下記を参考にvs2022/sqlite3に配置してください
               1. [SQLie公式サイト](https://www.sqlite.org/index.html)にアクセス
               1. [Download] クリック
               1. 「Precompiled Binaries for Windows」の sqlite-dll-win64-x64-3420000.zip をクリック（zipファイルがダウンロードされる）
               1. sqlite-dll-win64-x64-3420000.zip を解凍する
               1. 動作に必要なモジュールは sqlite3.dll です
               1. 以降は、ビルド環境で必要な作業です
                    - sqlite3.lib を作成する
                         - Visual Studio 2022 を起動
                         - [コードなしで続行] → [ツール] → [コマンドライン] → [開発者コマンドプロンプト]
                         - コマンドプロンプト上で以下を実行
                              ```
                              cd <sqlite-dll-win64-x64-3420000.zipの解凍ディレクトリ>
                              lib /def:sqlite3.def /machine:x64
                              ```
                         - sqlite3.lib と sqlite3.exp が作成される
                         - vs2022/sqlite3/lib/sqlite3.lib
                         - vs2022/sqlite3/dll/sqlite3.dll
                    - sqlite3.h を取得する
                         - [SQLie公式サイト](https://www.sqlite.org/index.html)にアクセス
                         - [Download] クリック
                         -「Source Code」の sqlite-amalgamation-3420000.zip をクリック（zipファイルがダウンロードされる）
                         - sqlite-amalgamation-3420000.zip を解凍する
                         - vs2022/sqlite3/include/sqlite3.h

     - ビルド設定
          - Windows
               - インクルードディレクトリ
                    - Visual Studio の該当プロジェクトの [プロパティページ] → [構成：すべての構成] → [C/C++] → [全般] → [追加のインクルードディレクトリ] で下記のパス名を追加してください。(複数の場合 ; で区切ってください。)
                         ```
                         <リポジトリ>/vs2022/sqlite3/include
                         ```
               - ライブラリディレクトリ
                    - Visual Studio の該当プロジェクトの [プロパティページ] → [構成：すべての構成] → [リンカー] → [全般] → [追加のライブラリディレクトリ] で下記のパス名を追加してください。(複数の場合 ; で区切ってください。)
                         ```
                         <リポジトリ>/vs2022/sqlite3/lib
                         ```
               - ライブラリ
                    - Visual Studio の該当プロジェクトの [プロパティページ] → [構成：すべての構成] → [リンカー] → [入力] → [追加の依存ファイル] で下記のファイル名を追加してください。(複数の場合 ; で区切ってください。)
                         ```
                         sqlite3.lib
                         ```

## 使い方

- インクルードディレクトリに　<リポジトリ>/src を追加してください
- spatial_api.h をインクルードしてください
     ```
     #include "spatial_api.h"
     ```

- テストプログラム
     ```
     <リポジトリ>/test/main.cpp
     ```
     - Windows
          - <リポジトリ>/vs2022/SpatialId.sln を Visual Studio 2022 で開いてください
