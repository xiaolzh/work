#include "common.h"
#include <assert.h>
#include <mysql/mysql.h>
#include <vector>
#include <string>
#include <fstream>
#include "util.h"
using namespace std; 

/// read data with sql config to data path, return true if succeed
bool ReadMysql(const SQLConfig& cfg, const string& data_path) {
    MYSQL* sql_ptr = mysql_init(NULL);
    /// - connect to sql
	if(sql_ptr == NULL) {
        ERROR("can't init mysql error info %s", mysql_error(sql_ptr));
		return false;
	}	

    int retry_cnt = 0;
	while(!mysql_real_connect(sql_ptr, cfg.host, cfg.user, cfg.pwd, 
        cfg.db, cfg.port, NULL, 0) && (retry_cnt++ < 3) ) {
		ERROR("Fail to connect mysql db:%s, error info %s, retry [%d]th...",
            cfg.db, mysql_error(sql_ptr), retry_cnt);
		sleep(2<<retry_cnt);
	}
    if(retry_cnt >=3 ) return false;
	if(mysql_query(sql_ptr, "set names 'gbk'")!=0) {
		ERROR("SQL Error: set names gbk error info %s", 
            mysql_error(sql_ptr));
		return false;
	}

	if(mysql_query(sql_ptr, cfg.sql)!=0) {
		ERROR("Sql Error: %s error info %s", cfg.sql, mysql_error(sql_ptr));
		return false;
	}	
    /// - get the fields and open field files for writing
	unsigned int num_fields;
	unsigned int i;
	MYSQL_ROW row;
	MYSQL_RES *result=mysql_use_result(sql_ptr);
	if(!result) {
		ERROR("Sql Error:");
		return false;
	}	

	vector<string> vFName;
	MYSQL_FIELD *field;
	while((field = mysql_fetch_field(result)))
		vFName.push_back(field->name);

	vector<vector<char> > vvBuf;
	FILE*fp;
	vector<FILE*> vfp;
	for(int i=0;i<vFName.size();++i) {
		if(!(fp=fopen((data_path+vFName[i]).c_str(),"w"))) {
			ERROR("error open file [%s]", vFName[i].c_str());
			return false;
		}
		vfp.push_back(fp);
		printf("field Name;%s\n",vFName[i].c_str());
	}
    /// - read field data and write into files
	int nLine=0;
	num_fields = mysql_num_fields(result);
	while ((row = mysql_fetch_row(result))) { 
		unsigned long *lengths; 
		lengths = mysql_fetch_lengths(result); 
		for(i = 0; i < num_fields; i++)  {  
			if((int)lengths[i]>0&&strlen(row[i])>0)
				fwrite(row[i],1,strlen(row[i])+1,vfp[i]);
			else
				fwrite("\0",1,1,vfp[i]);
		}
		if(nLine++%100==0) {
			printf("\rline: %d",nLine);
			fflush(stdout);
		}
	}
	printf("\n");
    /// - clean up
	for(int i=0;i<vfp.size();++i) fclose(vfp[i]);
	mysql_free_result(result);
	mysql_close(sql_ptr);
    return true;
}
 
/// opt help
static inline void Help() {
    printf( "Usage:\n"
            "  ./Reader [options] [args]\n"
            "Options:\n"
            "  -H      database host\n"
            "  -P      database port\n"
            "  -D      database used\n"
            "  -u      user\n"
            "  -p      password\n"
            "  -q      sql query\n"
            "  -d      data path for saving db data\n"
            "  -l      log path for saving logger infos\n"
            "  -h, -?  help\n"
        );
}

/// parse options to get reader's config
bool ParseOpt(int argc, char* argv[], ReaderConfig& cfg) {
    static const char* opt_str = "H:P:D:u:p:q:d:l:h?";
    int opt = getopt(argc, argv, opt_str);
    if( opt == -1) { Help();return false; }
    while( opt != -1 ) {
        switch( opt ) {
            case 'H':
                printf("db host: [%s]\n", optarg);
                cfg.sql_cfg.host = optarg;
                break;                
            case 'P':
                printf("db port: [%s]\n", optarg);
                cfg.sql_cfg.port = atoi(optarg);
                break;                
            case 'D':
                printf("db database: [%s]\n", optarg);
                cfg.sql_cfg.db = optarg;
                break;
            case 'u':
                printf("db user: [%s]\n", optarg);
                cfg.sql_cfg.user = optarg;
                break;                
            case 'p': 
                printf("db password: [%s]\n", optarg);
                cfg.sql_cfg.pwd = optarg;               
                break;      
            case 'q':
                printf("sql query: [%s]\n", optarg);
                cfg.sql_cfg.sql = optarg;
                break;       
            case 'd':
                printf("data path: [%s]\n", optarg);
                cfg.data_path = string(optarg) + string("/");
                break;
            case 'l':
                printf("log path: [%s]\n", optarg);
                cfg.log_path = string(optarg) + string("/");
                break;
            case 'h':   /* fall-through is intentional */
            case '?':
                Help();
                break;                
            default:
                /* You won't actually get here. */
                break;
        }        
        opt = getopt( argc, argv, opt_str);
    }    
    return true;
}

/// rewrite field "cat_paths" to generate new field "main_category"
static bool RewriteCatPaths(const string& data_path) {
    /// -- deal with the category path
    ifstream fin((data_path+"cat_paths").c_str());
    if(!fin) return true; /// need no rewrite
    ofstream fout((data_path+"main_category").c_str());
    if(!fout) {
        ERROR("error open file main_category");
        return false;
    }
    string cat, main_cat;
    while(true){
        getline(fin, cat, '\0');
        if(fin.eof()) break;  /// IGNORE the last '\0'
        main_cat = cat.substr(0, cat.find("|"));
        fout<<main_cat<<'\0';
    }
    fin.close();
    fout.close();
    return true;
}

/// rewrite field "isbn_search" as [isbn13,isbn10]
static bool RewriteISBN(const string& data_path) {
    /// -- deal with the ISBN
    /// @see rewrite 'standard_id' as 'isbn_search'
    //ifstream fin((data_path+"isbn_search").c_str());
    ifstream fin((data_path+"standard_id").c_str());
    if(!fin) return true; /// need no rewrite
    ofstream fout((data_path+"isbn_search.bak").c_str());
    if(!fout) {
        ERROR("error open file isbn_search.bak");
        return false;
    }
    string isbn, isbn2;
    while(true) {
        getline(fin, isbn, '\0');
        if(fin.eof()) break;  /// IGNORE the last '\0'
        isbn = isbn.substr(0, isbn.find("/"));
        if(isbn.size() == 10) {
            isbn2 = ISBN13(isbn);
            if(isbn2.empty()) { 
                /// Fail to convert isbn
                fout<<'\0';
            }else{
                fout<<isbn2<<','<<isbn<<'\0';
            }
        }else if(isbn.size() == 13) {
            isbn2 = ISBN10(isbn);
            if(isbn2.empty()) {
                /// fail to convert isbn
                fout<<'\0';
            }else{
                fout<<isbn<<','<<isbn2<<'\0';
            }
        }else{
            fout<<'\0';
        }
    }
    fin.close();
    fout.close();
    string mv_cmd = string("mv -f ") + data_path + string("isbn_search.bak ")
        + data_path + string("isbn_search");
    int ret = system(mv_cmd.c_str());
    if( ret != 0) {
        ERROR("Fail to cover file, cmd: [%s], error info: [%s]",
            mv_cmd.c_str(), strerror(errno));
        return false;
    }
    return true;
}

/// rewrite field "shop_info" to generate "shop_info_status", of which
/// the status 0 means info empty, 1 means not empty
bool RewriteShopInfo(const string& data_path) {
    /// -- deal with the shop_info
    ifstream fin( (data_path+"shop_info").c_str() );
    if( ! fin) return true; /// need no rewrite
    ofstream fout( (data_path+"shop_info_status").c_str() );
    if( ! fout) {
        ERROR("error open file shop_info_status");
        return false;
    }
    string shop_info;
    while( true ) {
        getline(fin, shop_info, '\0');
        if(fin.eof()) break;  /// IGNORE the last '\0'
        if( shop_info.empty() ) {
            fout<<0<<'\0';
        } else {
            fout<<1<<'\0';
        }
    }
    return true;
}

/// rewrite field "product_name" to generate "product_name_status", of which
/// the status 0 means info empty, 1 means not empty
bool RewriteProductName(const string& data_path) {
    /// -- deal with the shop_info
    ifstream fin( (data_path+"product_name").c_str() );
    if( ! fin) return true; /// need no rewrite
    ofstream fout( (data_path+"product_name_status").c_str() );
    if( ! fout) {
        ERROR("error open file shop_info_status");
        return false;
    }
    string product_name;
    while( true ) {
        getline(fin, product_name, '\0');
        if(fin.eof()) break;  /// IGNORE the last '\0'
        if( product_name.empty() ) {
            fout<<0<<'\0';
        } else {
            fout<<1<<'\0';
        }
    }
    return true;
}

static bool RemoveHtml(const string& field) {
#ifdef DEBUG
    struct timeval tvafter,tvpre;
    struct timezone tz;
    gettimeofday (&tvpre , &tz); 
#endif
    /// -- deal with the field need to remove html
    ifstream fin(field.c_str());
    if ( ! fin ) {
        return true; /// need no rewrite
    }
    FILE* fout = fopen( (field + ".bak").c_str(), "w");
    if( NULL ==  fout) {
        ERROR("Fail to open [%s].bak", field.c_str());
        return false;
    }
    string str;
    while(true) {
        getline(fin, str, '\0');
        if(fin.eof()) break;  /// IGNORE the last '\0'
        if ( ! str.empty() ) {
            RemoveHtmlStr(str);
            fwrite(&str[0], 1, str.size(), fout);
        }
        fwrite("\0", 1, 1, fout);
    }
    fclose(fout);
    fin.close();
    char cmd[256] = {0};
    snprintf(cmd, sizeof(cmd)-1, "mv -f %s.bak %s", field.c_str(),
        field.c_str());
    int ret = system(cmd);
    if ( 0 != ret) {
        ERROR("Fail to cover file [%s], ret [%d]", cmd, ret);
        return false;
    }
#ifdef DEBUG
    gettimeofday (&tvafter , &tz);
    ERROR("Remove html [%s] cost [%d] ms", field.c_str(),
        (tvafter.tv_sec-tvpre.tv_sec)*1000
        + (tvafter.tv_usec-tvpre.tv_usec)/1000 );
#endif 
    return true;
}

/// rewrite field "abstract" to remove html tags
bool RewriteAbstract(const string& data_path) {
    /// -- deal with the abstract
    ifstream fin( (data_path+"abstract").c_str() );
    if( ! fin) return true; /// need no rewrite
    return RemoveHtml(data_path + "abstract");
}

/// rewrite field "content" to remove html tags
bool RewriteContent(const string& data_path) {
    /// -- deal with the content
    ifstream fin( (data_path+"content").c_str() );
    if( ! fin) return true; /// need no rewrite
    return RemoveHtml(data_path + "content");
}

/// rewrite field "catalog" to remove html tags
bool RewriteCatalog(const string& data_path) {
    /// -- deal with the catalog
    ifstream fin( (data_path+"catalog").c_str() );
    if( ! fin) return true; /// need no rewrite
    return RemoveHtml(data_path + "catalog");
}

/// rewrite some special fields 
bool RewriteFields(const string& data_path) {
    if( ! RewriteCatPaths(data_path)) return false;
    if( ! RewriteISBN(data_path)) return false;
    if( ! RewriteShopInfo(data_path)) return false;
    if( ! RewriteProductName(data_path)) return false;
    if( ! RewriteAbstract(data_path)) return false;
    if( ! RewriteContent(data_path)) return false;
    if( ! RewriteCatalog(data_path)) return false;
    return true;
}

/// check the fields status and mark it 
bool CheckStatus(const string& data_path) {
    /// @todo check status in dispatcher, no here, for the reason that here
    ///       CAN'T check cross tables
    return true;
    ofstream fout( (data_path + "__llw_ok_status").c_str() );
    if( !fout) {
        ERROR("Error open file __llw_is_ok");
        return false;
    }
    ifstream fin( (data_path + "product_id").c_str() );
    if(!fin) return true; /// need no check
    ifstream price_in( (data_path + "dd_sale_price").c_str());
    ifstream name_in( (data_path + "product_name").c_str());
    ifstream is_publication_in( (data_path + "is_publication").c_str() );
    string id, price, name;
    int ok_status = 0; /// 0 means OK, 1 means NOT OK
    while(true) {
        ok_status = 0;
        getline(fin, id, '\0');
        if(fin.eof()) break;  /// IGNORE the last '\0'
        if( price_in) {
            /// check if the price is legal, which should > 0
            getline(price_in, price, '\0');
            if( atoi(price.c_str()) == 0)
                ok_status = 1;
        }
        if( name_in ) {
            /// check if the product_name is legal, which should not be empty
            getline(name_in, name, '\0');
            if( name.empty())
                ok_status = 1;
        }
        /// write the status to file
        fout<<ok_status<<'\0';
    }
    return true;
}
