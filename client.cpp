#include <kj/async-io.h>
#include <capnp/rpc-twoparty.h>
#include <capnp/message.h>
#include <capnp/serialize-packed.h>
#include <iostream>
#include <fstream>
#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include "image.capnp.h"

int main() {
    auto io = kj::setupAsyncIo();

    auto &waitScope = io.waitScope;
    kj::Network &network = io.provider->getNetwork();
    kj::Own<kj::NetworkAddress> addr = network.parseAddress("127.0.0.1:12000").wait(io.waitScope);
    kj::Own<kj::AsyncIoStream> conn = addr->connect().wait(io.waitScope);

    capnp::MallocMessageBuilder message;
    // msg type is ImageTransfer::TransferImageParams::Builder
    auto msg = message.initRoot<Image::TransferImageParams>();

    std::ifstream file("a.jpg", std::ios::binary | std::ios::ate);
    if (!file.is_open()) {
        std::cerr << "Failed to open image file.\n";
        return 1;
    }
    std::streamsize size = file.tellg();
    file.seekg(0, std::ios::beg);
    auto data = msg.initImage(size);
    file.read(reinterpret_cast<char*>(data.begin()), data.size());

    std::cout << "Size: " << size << "\n";

    capnp::TwoPartyClient client(*conn);
    Image::Client img = client.bootstrap().castAs<Image>();

    auto request = img.transferImageRequest();
    request.setImage(data);

    std::cout << "Sending request to server\n";
    auto promise = request.send();
    std::cout << "Request sent\n";

    auto readPromise = promise.then([](auto response) {
        auto imageResponse = response.getImage();
        return imageResponse;
    });

    if (file) {
        file.close();
    }

    std::cout << "Sent image back to server\n";
    auto response = readPromise.wait(waitScope);
    std::cout << "Received image with size: " << response.size() << " bytes\n";
    cv::Mat m(1, response.size(), CV_8UC1, const_cast<void *>(static_cast<const void *>(response.begin())));
    cv::Mat receivedImage = cv::imdecode(m, cv::IMREAD_UNCHANGED);
    if (receivedImage.empty()) {
      std::cerr << "Failed to decode image\n";
      return -1;
    }

    if (!cv::imwrite("rec.jpg", receivedImage)) {
        std::cerr << "Failed to save image\n";
        return -1;
    } 

    std::cout << "Image saved as rec.jpg\n";
    
    return 0;
}
