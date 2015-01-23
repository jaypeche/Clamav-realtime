/*
clamav-realtime.h
-----
This is ClamAV realtime scanner headers
*/

    void init_clam();
    void init_inotify();
    void recursive_loop();
    void scan();
    void notify();

//int rename(const char *old_filename, const char *new_filename);
