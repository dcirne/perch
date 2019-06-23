#include <grpcpp/grpcpp.h>
#include <iostream>
#include <map>
#include <memory>
#include <string>

#include "Messages_and_Services.pb.h"
#include "Messages_and_Services.grpc.pb.h"

class Server final : public Perch::Backend::Service {
    std::map<int, Perch::Interaction *> interactionMap;

public:
    grpc::Status interact(grpc::ServerContext *context, const Perch::Interaction *interaction, Perch::Acknowledge *acknowledge) override {
        Perch::Interaction *receivedInteraction = new Perch::Interaction();
        receivedInteraction->CopyFrom(*interaction);

        interactionMap[receivedInteraction->hour()] = receivedInteraction;

        std::cout << "Interaction\n" <<
                "\tDisplay: " << interaction->display().id() << "\n" <<
                "\tCustomer: " << interaction->customer().id() << "\n" <<
                "\tProduct: " << interaction->product().id() << "\n" <<
                "\tHour: " << interaction->hour() << "\n" <<
                std::endl;

        acknowledge->set_valid(true);
        return grpc::Status::OK;
    }
};

int main(int argc, char **argv) {
    const std::string serverAddress = "0.0.0.0:4242";
    Server server;

    grpc::ServerBuilder serverBuilder;
    serverBuilder.AddListeningPort(serverAddress, grpc::InsecureServerCredentials());
    serverBuilder.RegisterService(&server);

    std::unique_ptr<grpc::Server> gRPCServer(serverBuilder.BuildAndStart());
    std::cout << "Server ready." << std::endl;
    gRPCServer->Wait();

    return 0;
}
