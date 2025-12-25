#ifndef __REQUEST_H__

//void requestHandle(int fd);
void requestHandle(int fd, int* dynamic_req, int* static_req, int* total_req, int id, struct timeval* arrival,struct timeval* dispach , int* if_skip);

#endif
