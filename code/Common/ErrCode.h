#pragma once 
enum ERROR_TYPE{
	//��������
	enum_para_error = 9999,
   //�ļ�
	enum_data_count_error = 8888, //��֤��Сʧ��
	enum_failed_find_block_info=7777, //�����ļ���Ϣ�����
	enum_read_file_failed,
	enum_read_md5_error,
	enum_md5_failed,
	enum_file_write_failed,
	enum_set_blockinfo_failed,
	//����
	enum_timeout = 2001,
	enum_connect_failed=2002,
	enum_connect_success=2003,
	enum_success,
	enum_send_failed,
	enum_node_server_error, //�ڵ��������������
	enum_main_server_error,
	enum_node_server_normal_quit,
	enum_main_server_normal_quit,
	enum_datachannel_recv_success,
	enum_socket_pool_false,
	enum_client_quit, //�ͻ����˳�
	enum_ip_null,
	enum_unknown_failed,
	//
	enum_pkey_null,
	enum_write_data_larger_than_buffer_len,//д������ݿ鳬���ļ�������
	enum_client_interupt,// �ͻ�����ֹ
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