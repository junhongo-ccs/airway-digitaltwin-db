<!DOCTYPE html>
<html lang="ja">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <link href="{{ asset('css/main.css') }}" rel="stylesheet">
    @yield('head')
    <!-- CSRF Token -->
    <meta name="csrf-token" content="{{ csrf_token() }}">
    <title>{{ config('app.name', 'Laravel') }}</title>
</head>
<body>
    <header>
        <h1>{{ config('app.name', 'Laravel') }}</h1>
        @auth
            <form class="navbar" method="POST" action="{{ route('logout') }}">
                @csrf
                <button type="submit" class="btn-logout" >ログアウト</button>
            </form>
        @endauth
    <header>

    @yield('content')
</body>
</html>