@extends('layouts.app')

@section('content')

<main>
    <h2>HOME</h2>
    
    You are logged in!<br>

    <ul>
        <li>name: {{ Auth::user()->user_name }}</li>
        <li>group: {{ Auth::user()->group_id }}</li>
    </ul>
</main>
@endsection
