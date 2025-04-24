<?php

namespace App\Models;

use Illuminate\Database\Eloquent\Factories\HasFactory;
use Illuminate\Database\Eloquent\Model;

class DataSource extends Model
{
    use HasFactory;

    protected $table = 'data_sources';

    protected $primaryKey = 'data_source_id';

    public $incrementing = false;

}
