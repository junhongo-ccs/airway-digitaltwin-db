<?php

namespace App\Models;

use Illuminate\Database\Eloquent\Factories\HasFactory;
use Illuminate\Database\Eloquent\Model;

class ObjectMaster extends Model
{
    use HasFactory;

    protected $table = 'object_masters';

    protected $primaryKey = 'object_cd';

    public $incrementing = false;

    /**
     * The attributes that should be hidden for serialization.
     *
     * @var array<int, string>
     */
    //protected $hidden = [
    //];

    /**
     * The attributes that should be cast.
     *
     * @var array<string, string>
     */
    //protected $casts = [
    //];

    /**
     * The accessors to append to the model's array form.
     *
     * @var array<int, string>
     */
    //protected $appends = [,
    //];
}
