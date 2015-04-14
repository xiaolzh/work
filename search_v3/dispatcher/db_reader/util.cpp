#include "util.h"
#include <fcntl.h>

bool InitLogger(const std::string& log_file /*=""*/) {
    if( ! log_file.empty()) {
        int log_fd = -1;
        if( (log_fd = open(log_file.c_str(),  O_CREAT|O_RDWR|O_APPEND, 0600))
            == -1) {
            ERROR("Can't open log file [%s] for logging", log_file.c_str());
            return false;
        }
        fflush(stderr);
        dup2(log_fd, STDERR_FILENO);
        close(log_fd);
    }
    return true;
}

/// convert isbn13 to isbn10
std::string ISBN10(const std::string& isbn) {
   if(isbn.size()!=13) return "";
   if(isbn.substr(0,3) != "978") return "";
   std::string ret;
   int sum=0;
   for(int i=0;i<9;i++) {
      if(isbn[i+3]>='0' && isbn[i+3]<='9'){
        sum+=(isbn[i+3]-'0')*(10-i);
        ret+=isbn[i+3];
      }else return "";
   }
   sum %= 11;
   sum = 11-sum;
   ret += (sum==11?'0':(sum==10?'X':sum+'0'));
   return ret;
}

/// convert isbn10 to isbn13
std::string ISBN13(const std::string& isbn) {
  if(isbn.size() !=9 && isbn.size() !=10) return "";
  std::string ret="978";
  int sum=9*1+7*3+8*1;
  for(int i=0;i<9;i++) {
    if(isbn[i]>='0' && isbn[i]<='9') {
      sum+=(isbn[i]-'0')*(i%2?1:3);
      ret+=isbn[i];
    }else return "";
  }     
  if (sum%10==0) ret+='0';
  else ret += '0' + (10-sum%10);  
  return ret;
}

void RemoveHtmlStr(std::string& src) {
    std::string::size_type pos_beg, pos_end;
    pos_beg = src.find('<'); 
    while(std::string::npos != pos_beg) {
        pos_end = src.find('>', pos_beg + 1);
        if(std::string::npos != pos_end) {
            src.replace(pos_beg, pos_end + 1 - pos_beg, "");    
            pos_beg = src.find('<', pos_beg);
        } else {
            break;
        }
    }
}
