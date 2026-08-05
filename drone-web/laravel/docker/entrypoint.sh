#!/bin/sh
set -e

# Render Private Serviceはポートの動的割り当てを強制しない（コンテナが実際に
# 使っているポートをRenderが検出して内部ルーティングに反映する）。動的な
# ポート付け替えはミスの元になったため廃止し、Apache標準の80番に固定する。

# 仕様書§7-1〜§7-2: マイグレーションでテーブルを作成する。
# .envファイルは同梱せず、RenderのService環境変数から直接読む（Laravelは.env不在時は
# 実プロセス環境変数にフォールバックする）。
php artisan migrate --force

exec apache2-foreground
