<?php

namespace App\Models;

use Illuminate\Database\Eloquent\Factories\HasFactory;
use Illuminate\Database\Eloquent\Model;

class AveragePopulation extends Model
{
    use HasFactory;

    protected $table = 'average_population';

    protected $primaryKey = 'average_population_id';

}
