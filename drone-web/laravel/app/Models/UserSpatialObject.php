<?php

namespace App\Models;

use Illuminate\Database\Eloquent\Factories\HasFactory;
use Illuminate\Database\Eloquent\Model;

class UserSpatialObject extends Model
{
    use HasFactory;

    protected $table = 'user_object_masters';

    protected $fillable = [
        'group_id',  'user_object_name',
    ]; 

   protected $primaryKey = 'user_object_id';
}
