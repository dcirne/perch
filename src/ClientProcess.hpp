#include <grpcpp/grpcpp.h>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

#include "Messages_and_Services.pb.h"
#include "Messages_and_Services.grpc.pb.h"

class ClientProcess final {
    std::unique_ptr<Perch::Backend::Stub> computerStub;

public:
    ClientProcess(std::shared_ptr<grpc::Channel> channel) : computerStub(Perch::Backend::NewStub(channel)) {}

    bool interact(const Perch::Interaction &interaction) {
        grpc::ClientContext clientContext;
        Perch::Acknowledge acknowledge;
        grpc::Status status = computerStub->interact(&clientContext, interaction, &acknowledge);

        if (status.ok()) {
            return acknowledge.valid();
        } else {
            std::cout << "gRPC error. Code: " << status.error_code() << ", message: " << status.error_message() << std::endl;
            return false;
        }
    }
};
