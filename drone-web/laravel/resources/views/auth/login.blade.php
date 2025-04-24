@extends('layouts.app')

@section('head')
<link href="{{ asset('css/login.css') }}" rel="stylesheet">
@endsection('head')

@section('content')
<main>
    <h2>ログイン</h2>
    <form method="POST" action="{{ route('login') }}">
        @csrf
        <div>
            <input id="login-login_id" type="text" name="login_id" placeholder="User ID" value="{{ old('login_id') }}"/>
            <br><br>
            <input id="login-password" type="text" name="password" placeholder="Password" value="{{ old('password') }}"/>
            <br><br>
            <span>
            @foreach ($errors->all() as $error)
                {{ $error }}<br/>
            @endforeach
            </span>
        </div>

        <div>
            <button type="submit">ログイン</button>
        </div>
    </from>
</main>
@endsection