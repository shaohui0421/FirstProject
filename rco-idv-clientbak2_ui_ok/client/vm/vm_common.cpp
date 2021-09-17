#include "vm_common.h"

/*
**execute command and return output result
*/
string VM_Common::execute_command(string cmd)
{
    string cmd_result = "";
    char result[1024];
    int rc = 0;
    FILE *fp;
    fp = popen(cmd.c_str(),"r");
    if(fp == NULL)
    {
        perror("popen execute fail.");
        return "error";
    }
    while(fgets(result,sizeof(result),fp) != NULL)
    {
        string tempResult = result;
        cmd_result = cmd_result + tempResult;
    }
    rc = pclose(fp);
    if(rc == -1)
    {
        perror("close fp fail.");
        return "error";
    }else{
    	LOG_INFO("command:%s,subprocess end status:%d,command end status:%d",cmd.c_str(),rc,WEXITSTATUS(rc));

		if(WEXITSTATUS(rc) != 0)
		{
			return "error";
		}

		if(0 < cmd_result.length())
		{
			string tmp = cmd_result.substr(cmd_result.length() - 1, cmd_result.length());
			if(tmp == "\n" || tmp == "\r"){
				return cmd_result.substr(0, cmd_result.length() - 1) + "\0";
			}else{
				return cmd_result;
			}
		}else
		{
			return cmd_result;
		}

   }
}

