## XAMPPインストール
XAMPPを使用するか、PHP/Apache環境を構築してください。

## COMPOSERインストール
drone-webフォルダへ移動し composer install 

## DocumentRoot設定
DocumentRootは、drone-web/laravel/publicを設定してください。

## .env設定
local_envを参照し.envを作成してください<BR>
APP_KEYを作成・設定してください。<BR>
php artisan key:generate

## テーブル生成
php artisan migrate<BR>
