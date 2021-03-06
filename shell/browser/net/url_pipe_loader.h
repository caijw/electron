// Copyright (c) 2019 GitHub, Inc.
// Use of this source code is governed by the MIT license that can be
// found in the LICENSE file.

#ifndef SHELL_BROWSER_NET_URL_PIPE_LOADER_H_
#define SHELL_BROWSER_NET_URL_PIPE_LOADER_H_

#include <memory>
#include <string>
#include <vector>

#include "mojo/public/cpp/bindings/strong_binding.h"
#include "mojo/public/cpp/system/data_pipe_producer.h"
#include "services/network/public/cpp/simple_url_loader.h"
#include "services/network/public/cpp/simple_url_loader_stream_consumer.h"
#include "services/network/public/mojom/url_loader.mojom.h"

namespace network {
class SharedURLLoaderFactory;
}

namespace electron {

// Read data from URL and pipe it to NetworkService.
//
// Different from creating a new loader for the URL directly, protocol handlers
// using this loader can work around CORS restrictions.
//
// This class manages its own lifetime and should delete itself when the
// connection is lost or finished.
class URLPipeLoader : public network::mojom::URLLoader,
                      public network::SimpleURLLoaderStreamConsumer {
 public:
  URLPipeLoader(scoped_refptr<network::SharedURLLoaderFactory> factory,
                std::unique_ptr<network::ResourceRequest> request,
                network::mojom::URLLoaderRequest loader,
                network::mojom::URLLoaderClientPtr client,
                const net::NetworkTrafficAnnotationTag& annotation,
                base::DictionaryValue upload_data);

 private:
  ~URLPipeLoader() override;

  void Start(scoped_refptr<network::SharedURLLoaderFactory> factory,
             std::unique_ptr<network::ResourceRequest> request,
             const net::NetworkTrafficAnnotationTag& annotation,
             base::DictionaryValue upload_data);
  void NotifyComplete(int result);
  void OnResponseStarted(const GURL& final_url,
                         const network::ResourceResponseHead& response_head);
  void OnWrite(base::OnceClosure resume, MojoResult result);

  // SimpleURLLoaderStreamConsumer:
  void OnDataReceived(base::StringPiece string_piece,
                      base::OnceClosure resume) override;
  void OnComplete(bool success) override;
  void OnRetry(base::OnceClosure start_retry) override;

  // URLLoader:
  void FollowRedirect(const std::vector<std::string>& removed_headers,
                      const net::HttpRequestHeaders& modified_headers,
                      const base::Optional<GURL>& new_url) override {}
  void ProceedWithResponse() override {}
  void SetPriority(net::RequestPriority priority,
                   int32_t intra_priority_value) override {}
  void PauseReadingBodyFromNet() override {}
  void ResumeReadingBodyFromNet() override {}

  mojo::Binding<network::mojom::URLLoader> binding_;
  network::mojom::URLLoaderClientPtr client_;

  std::unique_ptr<mojo::DataPipeProducer> producer_;
  std::unique_ptr<network::SimpleURLLoader> loader_;

  base::WeakPtrFactory<URLPipeLoader> weak_factory_;

  DISALLOW_COPY_AND_ASSIGN(URLPipeLoader);
};

}  // namespace electron

#endif  // SHELL_BROWSER_NET_URL_PIPE_LOADER_H_
