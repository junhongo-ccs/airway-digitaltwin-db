<?php
namespace App\Console\Commands;
use Illuminate\Console\Command;
use App\Models\User;

class AddLoginUserCommand extends Command
{
    /**
     * The name and signature of the console command.
     *
     * @var string
     */
    protected $signature = 'user:add {group_id} {login_id} {password} {user_name}';

    /**
     * The console command description.
     *
     * @var string
     */
    protected $description = 'ログインユーザを追加するコマンド。user:add {group_id} {login_id} {password} {user_name}';

    /**
     * Create a new command instance.
     *
     * @return void
     */
    public function __construct()
    {
        parent::__construct();
    }

    /**
     * Execute the console command.
     *
     * @return mixed
     */
    public function handle()
    {
        echo '---------------------------------------------'.PHP_EOL;
        echo '>>> ユーザを追加します'.PHP_EOL;
        echo '---------------------------------------------'.PHP_EOL;
        $group_id   = $this->argument("group_id");
        $login_id   = $this->argument("login_id");
        $password   = $this->argument("password");
        $user_name  = $this->argument("user_name");
        echo 'group_id   :'.$group_id.PHP_EOL;
        echo 'login_id   :'.$login_id.PHP_EOL;
        echo 'password   :'.$password.PHP_EOL;
        echo 'user_name  :'.$user_name.PHP_EOL;

        $this->proc($group_id, $login_id, $password, $user_name);
        echo '---------------------------------------------'.PHP_EOL;
    }

    private function proc($group_id, $login_id, $password, $user_name) {

        // ID重複チェック
        $login_user = User::where('login_id', $login_id)->first();
        if($login_user) {
            echo '>>> 同じ login_id が登録されているため更新します。'.PHP_EOL;
        } else {
            $login_user = new User;
        }

        try {
            $login_user->group_id = $group_id;
            $login_user->login_id = $login_id;
            $login_user->password = bcrypt($password);
            $login_user->user_name = $user_name;

            $login_user->save();
        } catch (Exception $e) {
            \DB::rollBack();
            echo '>>> DBエラーが発生しました'.PHP_EOL;
            return;
        }
        echo '>>> 登録が正常に終了しました'.PHP_EOL;
    }
}