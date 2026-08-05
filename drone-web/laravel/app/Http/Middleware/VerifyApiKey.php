<?php

namespace App\Http\Middleware;

use Closure;
use Illuminate\Http\Request;
use Symfony\Component\HttpFoundation\Response;

/**
 * Sanctum本格導入までの暫定措置（junhongo-ccs/airspace docs/ドローン航路GIS-PoC_仕様書.md §9）。
 * 環境変数 API_KEY と、リクエストヘッダー X-API-Key を比較するだけの簡易な認証。
 * さらにこのサービス自体をRenderのPrivate Serviceとして配備し、外部からの直接到達を防ぐ
 * ことをあわせて前提とする（本ミドルウェア単体でネットワーク制限の代替にはしない）。
 */
class VerifyApiKey
{
    public function handle(Request $request, Closure $next): Response
    {
        $expected = env('API_KEY');
        $provided = $request->header('X-API-Key');

        if (empty($expected) || !is_string($provided) || !hash_equals($expected, $provided)) {
            return response()->json(['message' => 'Unauthorized'], 401);
        }

        return $next($request);
    }
}
