#ifndef HTTP_CLASS_H
#define HTTP_CLASS_H

HTTP_NAMESPACE_BEGIN

class HTTPClass
{
 public:
	std::vector<Sinkhole::cidr> sources;
	HTTPAction *action;

	HTTPClass(HTTPAction *a);
};

HTTP_NAMESPACE_END

#endif // HTTP_CLASS_H

