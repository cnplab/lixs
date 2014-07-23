#include <lixs/client.hh>


lixs::client::client(iomux& io)
    : io(io)
{
    io.once(*this);
}

lixs::client::~client()
{
}


void lixs::client::run(void)
{
}

void lixs::client::handle(const ioev& events)
{
}

