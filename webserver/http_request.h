#define MAX_TARGET_SIZE 1024

enum http_method {
	HTTP_GET,
	HTTP_UNSUPPORTED,
};

typedef struct {
	enum http_method method;
	int major_version;
	int minor_version;
	char target[MAX_TARGET_SIZE];
} http_request;