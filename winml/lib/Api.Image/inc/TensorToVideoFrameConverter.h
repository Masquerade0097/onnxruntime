// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma once

#include "ImageConverter.h"
#include "ImageConversionTypes.h"

namespace Windows::AI::MachineLearning::Internal {
class ITensorToVideoFrameConverter {
 public:
  virtual void DX12TensorToVideoFrame(
      _In_ UINT32 batch_index,
      _In_ winrt::Windows::AI::MachineLearning::LearningModelSession& session,
      _In_ ID3D12Resource* input_tensor,
      _In_ const ImageTensorDescription& tensor_description,
      _Inout_ winrt::Windows::Media::VideoFrame& destination_video_frame) = 0;

  virtual void SoftwareTensorToVideoFrame(
      _In_ winrt::Windows::AI::MachineLearning::LearningModelSession& session,
      _In_ BYTE* CPU_tensor_to_convert,
      _In_ ImageTensorDescription tensor_description,
      _Inout_ winrt::Windows::Media::VideoFrame& destination_video_frame) = 0;
};

class TensorToVideoFrameConverter : ITensorToVideoFrameConverter, public ImageConverter {
 public:
  TensorToVideoFrameConverter() : shared_handle_(nullptr) {}

  // Function takes in a tensor DX12 Resource all compute ops should be completed
  // converts it to a VideoFrame backed by either a SoftwareBitmap or D3DSurface
  void DX12TensorToVideoFrame(
      _In_ UINT32 batch_index,
      _In_ winrt::Windows::AI::MachineLearning::LearningModelSession& session,
      _In_ ID3D12Resource* input_tensor,
      _In_ const ImageTensorDescription& tensor_description,
      _Inout_ winrt::Windows::Media::VideoFrame& destination_video_frame);

  // Function takes in a byte pointer to a CPUTensor
  // converts it to VideoFrame backed by either a SoftwareBitmap or D3DSurface,
  void SoftwareTensorToVideoFrame(
      _In_ winrt::Windows::AI::MachineLearning::LearningModelSession& session,
      _In_ BYTE* CPU_tensor_to_convert,
      _In_ ImageTensorDescription tensor_description,
      _Inout_ winrt::Windows::Media::VideoFrame& destination_video_frame);

 private:
  GUID _d3d11TextureGUID = {0x14bf1054, 0x6ce7, 0x4c00, {0xa1, 0x32, 0xb0, 0xf2, 0x11, 0x5D, 0xE0, 0x7f}};  // {14BF1054-6CE7-4C00-A132-B0F2115DE07F}
  GUID _handleGUID = {0x700148fc, 0xc0cb, 0x4a7e, {0xa7, 0xc0, 0xe7, 0x43, 0xc1, 0x9, 0x9d, 0x62}};
  ;  // {700148FC-C0CB-4A7E-A7C0-E743C1099D62}
  Microsoft::WRL::ComPtr<ID3D12Resource> readback_heap_;
  Microsoft::WRL::ComPtr<ID3D12Resource> output_resource_;
  Microsoft::WRL::ComPtr<ID3D12Resource> UAV_resource_;
  HANDLE shared_handle_;

  Microsoft::WRL::ComPtr<ID3D11Texture2D> ShareD3D12Texture(ID3D12Resource* pResource, ID3D11Device* pDevice);

  void ConvertGPUTensorToSoftwareBitmap(
      _In_ UINT32 batch_index,
      _In_ ID3D12Resource* input_tensor,
      _In_ winrt::Windows::AI::MachineLearning::implementation::D3DDeviceCache& device_cache,
      _In_ const ImageTensorDescription& tensor_description,
      _Inout_ winrt::Windows::Graphics::Imaging::SoftwareBitmap& software_bitmap);

  void ConvertGPUTensorToDX12Texture(
      _In_ UINT32 batch_index,
      _In_ ID3D12Resource* input_resource,
      _In_ winrt::Windows::AI::MachineLearning::implementation::D3DDeviceCache& device_cache,
      _In_ const ImageTensorDescription& tensor_description,
      _Inout_ ID3D12Resource* output_resource);

  void ConvertDX12TensorToUnsupportedVideoFrameFormat(
      _In_ UINT32 batch_index,
      _In_ ID3D12Resource* input_tensor,
      _In_ winrt::Windows::AI::MachineLearning::implementation::D3DDeviceCache& device_cache,
      _In_ const ImageTensorDescription& tensor_description,
      _Inout_ winrt::Windows::Media::VideoFrame& unsupported_video_frame);

  static D3D12_SHADER_RESOURCE_VIEW_DESC TensorToVideoFrameConverter::CreateSRVDescriptor(
      const UINT32 batch_index,
      const D3D12_RESOURCE_DESC& resource_description,
      const ImageTensorDescription& description);

  static void ConvertCPUTensorToSoftwareBitmap(
      _In_ void* CPU_tensor,
      _In_ const ImageTensorDescription& tensor_description,
      _Inout_ winrt::Windows::Graphics::Imaging::SoftwareBitmap& software_bitmap);

  static Microsoft::WRL::ComPtr<ID3D12Resource> CreateShareableD3D12Texture(
      const D3D11_TEXTURE2D_DESC& d3d11Desc,
      ID3D12Device* d3d12Device);
};
}  // namespace Windows::AI::MachineLearning::Internal