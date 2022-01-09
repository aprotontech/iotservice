<?php

namespace App\Http\Controllers\Admin;

use App\Models\User;
use Illuminate\Http\Request;
use Illuminate\Pagination\Paginator;

class UserListController extends \App\Http\Controllers\WebController
{
    public $permission = 'admin_manager';

    use \Proton\Framework\ApiControllerTrait;

    public function getFilter()
    {
        return [
            "account" => "optional | is_string",
            "page" => "optional | is_int",
            "size" => "optional | is_int",
        ];
    }

    public function handle(Request $req)
    {
        $reqinfo = $this->reqToObject($req);

        $account = isset($reqinfo->account) ? trim($reqinfo->account) : '';
        $page = isset($reqinfo->page) ? intval($reqinfo->page) : 1;
        $size = isset($reqinfo->size) ? intval($reqinfo->size) : 20;

        Paginator::currentPageResolver(function () use ($page) {
            return $page;
        });
        $userList = User::master($account)
            ->orderBy('id', 'desc')
            ->paginate($size);

        return $this->formatData($userList);
    }

    private function formatData($userList)
    {
        $data = ['total' => 0, 'list' => []];

        if ($userList) {
            $data['total'] = $userList->total();
            foreach ($userList as $user) {
                $row = [];
                $row['id'] = $user->id;
                $row['name'] = $user->name;
                $row['account'] = $user->email;
                $row['phone'] = '';
                $row['email'] = $user->email;
                $row['company'] = '';
                $row['state'] = $user->deleted_at ? 0 : 1;
                $row['createdAt'] = $user->created_at->toDateTimeString();
                $row['updatedAt'] = $user->updated_at->toDateTimeString();
                $row['deletedAt'] = $user->deleted_at;
                $data['list'][] = $row;
            }
        }

        return $this->success($data);
    }
}
