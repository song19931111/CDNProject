#pragma once 
enum ERROR_TYPE{
	//参数错误
	enum_para_error = 9999,
   //文件
	enum_data_count_error = 8888, //验证大小失败
	enum_failed_find_block_info=7777, //查找文件信息块错误
	enum_read_file_failed,
	enum_read_md5_error,
	enum_md5_failed,
	enum_file_write_failed,
	enum_set_blockinfo_failed,
	//网络
	enum_timeout = 2001,
	enum_connect_failed=2002,
	enum_connect_success=2003,
	enum_success,
	enum_send_failed,
	enum_node_server_error, //节点服务器发生错误
	enum_main_server_error,
	enum_node_server_normal_quit,
	enum_main_server_normal_quit,
	enum_datachannel_recv_success,
	enum_socket_pool_false,
	enum_client_quit, //客户端退出
	enum_ip_null,
	enum_unknown_failed,
	//
	enum_pkey_null,
	enum_write_data_larger_than_buffer_len,//写入的数据块超出文件的限制
	enum_client_interupt,// 客户端终止
	//HTTP:
	enum_get_file_size_from_http_failed,

	enum_nothing_find_in_downloadling_list =5555
};
class CErrCode{
public:
	char szResult[100];
	
public:
	CErrCode();
	~CErrCode();	
	char * GetReSult(int errCode);
};