#include <pqxx/pqxx>
#include <Magic/Magic>
#include <VodServices.h>

int main(int argc, char** argv){
    Magic::Thread::SetName("Main");
    VodServices::Initialize([](const Safe<Magic::Container>& ioc){
        ioc->registerType<Magic::DataBase::ConnectionPool<pqxx::connection>,Magic::Config,Magic::TimingWheel>();
    });
    return EXIT_SUCCESS;
}