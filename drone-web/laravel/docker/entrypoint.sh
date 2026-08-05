#!/bin/sh
set -e

# RenderはPORT環境変数でリッスンすべきポートを指定してくる（既定10000想定だが固定しない）。
PORT="${PORT:-10000}"
sed -i "s/10000/${PORT}/g" /etc/apache2/ports.conf /etc/apache2/sites-available/000-default.conf

# 仕様書§7-1〜§7-2: マイグレーションでテーブルを作成する。
# .envファイルは同梱せず、RenderのService環境変数から直接読む（Laravelは.env不在時は
# 実プロセス環境変数にフォールバックする）。
php artisan migrate --force

exec apache2-foreground
