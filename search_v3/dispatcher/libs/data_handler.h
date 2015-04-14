#ifndef DATA_HANDLER_H
#define DATA_HANDLER_H

#ifdef __cplusplus
extern "C" {
#endif

/// Initialization, here usually init the data handler with config file. 
/// return 0 if succeed
int init(const char* config);

/// Counter initialization
/// return 0 if succeed
int co_init();

/** process_data() is used to process data, usually get data from database,
 *  handle the data and build data for searching.
 *  This function is called to process the module data. If succeed to do 
 *  main process or inc process, it return 0.
 *  process_type: "main" or "inc"
 */
int process_data(const char* process_type);

/** get the inc data
 *  @paras  inc_stamp - the inc time stamp of data
 *          data_buf - the buffer to get inc data, which is allocated by 
 *                     caller, if NULL is passed in, the function will just
 *                     return the data length of inc data
 *          data_len - the length of data buffer passed in
 *  return  the actually length of inc data, if -1 means no data
 *  
 */
int get_inc_data(int inc_stamp, void* data_buf, int data_len);


/** get the full data
 *  @paras key - the key of the full data stored in data table
 *         data_buf - the buffer to get full data, which is allocated by 
 *                    caller, if the buf size is too small to save the data,
 *                    the data info will be cut off, just like snprintf()
 *         data_len - the length of data buffer passed in
 *  return the actually length of inc data, if -1 means no data
 */
int get_full_data(const char* key, void* data_buf, int data_len);

/// return the inc stamp
int get_inc_stamp();

/// return the update status, 0 means need do main process, 1 means no need
int get_update_status();

/// return the data path, with '\0' at end
int get_data_path(char* path_buffer, int buffer_len);

#ifdef __cplusplus
}
#endif 

#endif // ~>.!.<~
