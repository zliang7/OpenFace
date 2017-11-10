typedef void JSNIEnv;
typedef void (*AsyncThreadWorkCallback)(JSNIEnv* env, void*);
typedef void (*AsyncThreadWorkAfterCallback)(JSNIEnv* env, void*);
void AsyncThreadWork(JSNIEnv* env, void* data,
                     AsyncThreadWorkCallback work,
                     AsyncThreadWorkAfterCallback callback) {}
