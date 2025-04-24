<?php

namespace App\Http\Controllers\Api;

use Illuminate\Http\Request;
use Illuminate\Support\Facades\Validator;
use Illuminate\Support\Facades\Auth;
use App\Http\Controllers\Controller;

class UserController extends Controller
{
    public const TOKEN_NAME = 'api_token';

    public function login(Request $request)
    {
        $validator = Validator::make($request->all(), [
            'login_id' => ['required'],
            'password' => ['required'],
        ]);
        if ($validator->fails()) {
            return response()->json(['api_token' => null], 401);
        }

        // バリデーション済みデータの取得
        $credentials = $validator->validated();
        if (Auth::attempt($credentials)) {
            $request->user()->tokens()->delete();
            $token = $request->user()->createToken(self::TOKEN_NAME, ['*'], now()->addDay());
            return response()->json([
                'api_token' => $token->plainTextToken,
                'expires_at' => $token->accessToken->expires_at
            ]);
        }
        return response()->json(['api_token' => null], 401);
    }

    public function logout(Request $request)
    {
        $request->user()->tokens()->delete();
    }
}