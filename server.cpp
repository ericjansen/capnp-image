#include <iostream>
#include <fstream>
#include <capnp/rpc-twoparty.h>
#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include "image.capnp.h"

class ImageTransferImpl final : public Image::Server
{
public:
    kj::Promise<void> transferImage(TransferImageContext context) override
    {
        auto img = context.getParams().getImage();
        auto size = img.size();

        std::cout << "Received image with size: " << size << " bytes\n";
        cv::Mat m(1, size, CV_8UC1, const_cast<void *>(static_cast<const void *>(img.begin())));
        cv::Mat receivedImage = cv::imdecode(m, cv::IMREAD_COLOR);
        cv::Mat grayscaleImg;
        cv::cvtColor(receivedImage, grayscaleImg, cv::COLOR_BGR2GRAY);

        std::vector<uchar> data;
        cv::imencode(".jpg", grayscaleImg, data);
        cv::imwrite("grayscale.jpg", grayscaleImg);
        std::cout << "Image saved as grayscale.jpg\n";
        auto response = context.getResults(); //.initImage();
        auto imgBuilder = response.initImage(data.size());
        std::memcpy(imgBuilder.begin(), data.data(), data.size());

        return kj::READY_NOW;
    }
};

int main()
{
    auto io = kj::setupAsyncIo();
    kj::Network &network = io.provider->getNetwork();
    kj::Own<kj::NetworkAddress> addr = network.parseAddress("127.0.0.1:12000").wait(io.waitScope);
    kj::Own<kj::ConnectionReceiver> listener = addr->listen();

    capnp::TwoPartyServer server(kj::heap<ImageTransferImpl>());

    uint port = listener->getPort();
    std::cout << "Listening on port " << port << std::endl;

    server.listen(*listener).wait(io.waitScope);

    return 0;
}
