<?php

namespace App\Models;

use Illuminate\Database\Eloquent\Factories\HasFactory;
use Illuminate\Database\Eloquent\Model;
use Illuminate\Database\Eloquent\SoftDeletes;

class SpacialDetailAttributeFile extends Model
{
    use HasFactory;

    protected $table = 'spacial_detail_attribute_files';
    protected $guarded = [];

    use SoftDeletes;

    protected $dates = ['deleted_at'];
}
