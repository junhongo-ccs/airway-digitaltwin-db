# Render Private Service向けDockerfile。
# 参照: junhongo-ccs/airspace の docs/実装タスクリスト.md §1、render.yaml
#
# Renderの「Root Directory」= drone-web/laravel を前提に、このファイル自身の
# ディレクトリをビルドコンテキストとする（COPYはすべてこのディレクトリ基準）。
FROM php:8.2-apache

RUN apt-get update && apt-get install -y --no-install-recommends \
        libzip-dev \
        libpng-dev \
        libonig-dev \
        libxml2-dev \
        unzip \
        git \
        curl \
    && docker-php-ext-install pdo pdo_mysql mbstring bcmath zip gd \
    && a2enmod rewrite \
    && rm -rf /var/lib/apt/lists/*

COPY --from=composer:2 /usr/bin/composer /usr/bin/composer

WORKDIR /var/www/html

# 依存関係だけ先にコピーしてcomposer installをキャッシュさせる
COPY composer.json composer.lock ./
RUN composer install --no-dev --no-scripts --no-interaction --prefer-dist --optimize-autoloader

COPY . .

RUN composer dump-autoload --optimize --no-scripts \
    && mkdir -p storage/framework/cache storage/framework/sessions storage/framework/views storage/logs bootstrap/cache \
    && chown -R www-data:www-data storage bootstrap/cache

COPY docker/apache-vhost.conf /etc/apache2/sites-available/000-default.conf
COPY docker/entrypoint.sh /entrypoint.sh
RUN chmod +x /entrypoint.sh

EXPOSE 10000
ENTRYPOINT ["/entrypoint.sh"]
