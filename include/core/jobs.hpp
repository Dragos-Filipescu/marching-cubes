#pragma once
#ifndef MARCHING_CUBES_CORE_JOBS_HPP
#define MARCHING_CUBES_CORE_JOBS_HPP

#include <vulkan/vulkan_core.h>

#include <concepts>
#include <cstdint>
#include <functional>
#include <print>
#include <type_traits>
#include <unordered_map>
#include <utility>
#include <vector>

#include <core/aliases.hpp>
#include <core/buffer.hpp>
#include <core/image.hpp>
#include <core/physical_device.hpp>

#include <utils/utils.hpp>

namespace marching_cubes::core::jobs {

	static inline constexpr auto kNoop = []() {};

	class IJob {
	public:
		virtual IJob& record(VkCommandBuffer commandBuffer) = 0;
		virtual void onEnqueue() const = 0;
		virtual void onComplete() const = 0;
		virtual ~IJob() = default;
	};

	template<
		std::invocable OnEnqueue = decltype(kNoop),
		std::invocable OnComplete = decltype(kNoop)
	>
	class BufferTransferJob final : public IJob {
	public:
		BufferTransferJob(
			const Buffer& src,
			const Buffer& dst,
			std::vector<VkBufferCopy> copyRegions,
			OnEnqueue&& onEnqueueCallback,
			OnComplete&& onCompleteCallback
		)
			: m_Src{ src },
			m_Dst{ dst },
			m_CopyRegions{ std::move(copyRegions) },
			m_OnEnqueueCallback{ std::forward<OnEnqueue>(onEnqueueCallback) },
			m_OnCompleteCallback{ std::forward<OnComplete>(onCompleteCallback) }
		{}

		BufferTransferJob(
			const Buffer& src,
			const Buffer& dst,
			std::vector<VkBufferCopy> copyRegions
		)
			: m_Src{ src },
			m_Dst{ dst },
			m_CopyRegions{ std::move(copyRegions) },
			m_OnEnqueueCallback{ std::move(kNoop) },
			m_OnCompleteCallback{ std::move(kNoop) }
		{}

		BufferTransferJob(const BufferTransferJob&) = default;
		BufferTransferJob& operator=(const BufferTransferJob&) = default;

		BufferTransferJob(BufferTransferJob&&) = default;
		BufferTransferJob& operator=(BufferTransferJob&&) = default;

		virtual ~BufferTransferJob() = default;

		BufferTransferJob& record(VkCommandBuffer commandBuffer) override
		{
			vkCmdCopyBuffer(
				commandBuffer,
				m_Src.get(),
				m_Dst.get(),
				static_cast<u32>(m_CopyRegions.size()),
				m_CopyRegions.data()
			);
			return *this;
		}

		void onEnqueue() const override {
			m_OnEnqueueCallback();
		}
		void onComplete() const override {
			m_OnCompleteCallback();
		}
		
	private:
		std::reference_wrapper<const Buffer>	m_Src;
		std::reference_wrapper<const Buffer>	m_Dst;
		std::vector<VkBufferCopy>				m_CopyRegions;
		OnEnqueue								m_OnEnqueueCallback;
		OnComplete								m_OnCompleteCallback;
	};

	template<
		std::invocable OnEnqueue = decltype(kNoop),
		std::invocable OnComplete = decltype(kNoop)
	>
	class ImageLayoutTransitionJob final : public IJob {
	public:
		ImageLayoutTransitionJob(
			Image& image,
			VkImageLayout newLayout,
			const VkImageSubresourceRange& subresourceRange,
			OnEnqueue&& onEnqueueCallback,
			OnComplete&& onCompleteCallback
		)
			: m_Image{ image },
			m_NewLayout{ newLayout },
			m_SubresourceRange{ subresourceRange },
			m_OnEnqueueCallback{ std::forward<OnEnqueue>(onEnqueueCallback) },
			m_OnCompleteCallback{ std::forward<OnComplete>(onCompleteCallback) }
		{}

		ImageLayoutTransitionJob(
			Image& image,
			VkImageLayout newLayout,
			const VkImageSubresourceRange& subresourceRange
		)
			: m_Image{ image },
			m_NewLayout{ newLayout },
			m_SubresourceRange{ subresourceRange },
			m_OnEnqueueCallback{ std::move(kNoop) },
			m_OnCompleteCallback{ std::move(kNoop) }
		{}

		ImageLayoutTransitionJob(const ImageLayoutTransitionJob&) = default;
		ImageLayoutTransitionJob& operator=(const ImageLayoutTransitionJob&) = default;

		ImageLayoutTransitionJob(ImageLayoutTransitionJob&&) = default;
		ImageLayoutTransitionJob& operator=(ImageLayoutTransitionJob&&) = default;

		virtual ~ImageLayoutTransitionJob() = default;

		ImageLayoutTransitionJob& record(VkCommandBuffer commandBuffer) override {

			auto& img = m_Image.get();
			const auto& createInfo = img.getCreateInfo();

			for (u32 mipLevel = m_SubresourceRange.baseMipLevel;
				mipLevel < m_SubresourceRange.baseMipLevel + m_SubresourceRange.levelCount;
				mipLevel++
			) {
				const auto& oldLayout = img.getCurrentLayouts().at(mipLevel);

				if (oldLayout == m_NewLayout) {
					img.setCurrentLayout(mipLevel, m_NewLayout);
					continue;
				}

				const auto& [
					finalBarrier,
					sourceStage,
					destinationStage
				] = prepareImageLayoutTransition(
					img,
					createInfo,
					oldLayout,
					m_NewLayout,
					VkImageSubresourceRange{
						.aspectMask = m_SubresourceRange.aspectMask,
						.baseMipLevel = mipLevel,
						.levelCount = 1,
						.baseArrayLayer = m_SubresourceRange.baseArrayLayer,
						.layerCount = m_SubresourceRange.layerCount,
					}
				);

				vkCmdPipelineBarrier(
					commandBuffer,
					sourceStage,
					destinationStage,
					0,
					0,
					nullptr,
					0,
					nullptr,
					1,
					&finalBarrier
				);

				img.setCurrentLayout(mipLevel, m_NewLayout);
			}

			return *this;
		}

		void onEnqueue() const override {
			m_OnEnqueueCallback();
		}
		void onComplete() const override {
			m_OnCompleteCallback();
		}

	private:
		std::reference_wrapper<Image>	m_Image;
		VkImageLayout					m_NewLayout;
		VkImageSubresourceRange			m_SubresourceRange;
		OnEnqueue						m_OnEnqueueCallback;
		OnComplete						m_OnCompleteCallback;
	};

	template<
		std::invocable OnEnqueue = decltype(kNoop),
		std::invocable OnComplete = decltype(kNoop)
	>
	class ImageTransferJob final : public IJob {
	public:
		ImageTransferJob(
			const Buffer& buffer,
			Image& image,
			const VkBufferImageCopy& bufferCopy,
			OnEnqueue&& onEnqueueCallback,
			OnComplete&& onCompleteCallback
		)
			: m_Buffer{ buffer },
			m_Image{ image },
			m_BufferCopy{ bufferCopy },
			m_OnEnqueueCallback{ std::forward<OnComplete>(onEnqueueCallback) },
			m_OnCompleteCallback{ std::forward<OnComplete>(onCompleteCallback) }
		{}

		ImageTransferJob(
			Buffer& buffer,
			Image& image,
			const VkBufferImageCopy& bufferCopy
		)
			: m_Buffer{ buffer },
			m_Image{ image },
			m_BufferCopy{ bufferCopy },
			m_OnEnqueueCallback{ std::move(kNoop) },
			m_OnCompleteCallback{ std::move(kNoop) }
		{}

		ImageTransferJob(const ImageTransferJob&) = default;
		ImageTransferJob& operator=(const ImageTransferJob&) = default;

		ImageTransferJob(ImageTransferJob&&) = default;
		ImageTransferJob& operator=(ImageTransferJob&&) = default;

		virtual ~ImageTransferJob() = default;

		ImageTransferJob& record(VkCommandBuffer commandBuffer) override {
			vkCmdCopyBufferToImage(
				commandBuffer,
				m_Buffer.get(),
				m_Image.get(),
				VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
				1,
				&m_BufferCopy
			);
			return *this;
		}

		void onEnqueue() const override {
			m_OnEnqueueCallback();
		}
		void onComplete() const override {
			m_OnCompleteCallback();
		}

	private:
		std::reference_wrapper<const Buffer>	m_Buffer;
		std::reference_wrapper<Image>			m_Image;
		VkBufferImageCopy						m_BufferCopy;
		OnEnqueue								m_OnEnqueueCallback;
		OnComplete								m_OnCompleteCallback;
	};

	template<
		std::invocable OnEnqueue = decltype(kNoop),
		std::invocable OnComplete = decltype(kNoop)
	>
	class ImageBlitJob final : public IJob {
	public:
		ImageBlitJob(
			Image& image,
			OnEnqueue&& onEnqueueCallback,
			OnComplete&& onCompleteCallback
		)
			: m_Image{ image },
			m_OnEnqueueCallback{ std::forward<OnEnqueue>(onEnqueueCallback) },
			m_OnCompleteCallback{ std::forward<OnComplete>(onCompleteCallback) }
		{}

		ImageBlitJob(
			Image& image
		)
			: m_Image{ image },
			m_OnEnqueueCallback{ kNoop },
			m_OnCompleteCallback{ kNoop }
		{}

		ImageBlitJob(const ImageBlitJob&) = default;
		ImageBlitJob& operator=(const ImageBlitJob&) = default;

		ImageBlitJob(ImageBlitJob&&) = default;
		ImageBlitJob& operator=(ImageBlitJob&&) = default;

		virtual ~ImageBlitJob() = default;

		ImageBlitJob& record(VkCommandBuffer commandBuffer) override {

			auto& img = m_Image.get();
			const auto& createInfo = img.getCreateInfo();

			i32 mipWidth = createInfo.extent.width;
			i32 mipHeight = createInfo.extent.height;

			std::unordered_map<VkImageSubresource, VkImageLayout, VkImageSubresourceHasher> m_SubresourceLayouts;

			auto transition_mip = [&](u32 mipLevel, VkImageLayout newLayout) {
				ImageLayoutTransitionJob{
					m_Image,
					newLayout,
					{
						.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
						.baseMipLevel = mipLevel,
						.levelCount = 1,
						.baseArrayLayer = 0,
						.layerCount = 1,
					}
				}.record(commandBuffer);
				m_Image.get().setCurrentLayout(mipLevel, newLayout);
			};

			for (u32 i = 1; i < createInfo.mipLevels; i++) {

				transition_mip(i - 1, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
				transition_mip(i, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

				VkImageBlit blit{
					.srcSubresource {
						.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
						.mipLevel = i - 1,
						.baseArrayLayer = 0,
						.layerCount = createInfo.arrayLayers,
					},
					.srcOffsets = {
						{ 0, 0, 0 },
						{ mipWidth, mipHeight, 1 },
					},
					.dstSubresource = {
						.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
						.mipLevel = i,
						.baseArrayLayer = 0,
						.layerCount = createInfo.arrayLayers,
					},
					.dstOffsets = {
						{ 0, 0, 0 },
						{ mipWidth > 1 ? (mipWidth >> 1) : 1, mipHeight > 1 ? (mipHeight >> 1) : 1, 1 },
					},
				};

				vkCmdBlitImage(
					commandBuffer,
					img,
					VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
					img,
					VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
					1,
					&blit,
					VK_FILTER_LINEAR
				);
				
				mipWidth = mipWidth > 1 ? (mipWidth >> 1) : 1;
				mipHeight = mipHeight > 1 ? (mipHeight >> 1) : 1;
			}

			return *this;
		}

		void onEnqueue() const override {
			m_OnEnqueueCallback();
		}
		void onComplete() const override {
			m_OnCompleteCallback();
		}

	private:
		std::reference_wrapper<Image>	m_Image;
		OnEnqueue						m_OnEnqueueCallback;
		OnComplete						m_OnCompleteCallback;
	};

	template<
		std::invocable OnEnqueue,
		std::invocable OnComplete
	>
	BufferTransferJob(
		const Buffer&,
		const Buffer&,
		VkBufferCopy,
		OnEnqueue&&,
		OnComplete&&
	) -> BufferTransferJob<OnEnqueue, OnComplete>;

	template<
		std::invocable OnEnqueue,
		std::invocable OnComplete
	>
	ImageLayoutTransitionJob(
		Image&,
		VkImageLayout,
		const VkImageSubresourceRange&,
		OnEnqueue&&,
		OnComplete&&
	) -> ImageLayoutTransitionJob<OnEnqueue, OnComplete>;

	template<
		std::invocable OnEnqueue,
		std::invocable OnComplete
	>
	ImageTransferJob(
		const Buffer&,
		Image&,
		const VkBufferImageCopy&,
		OnEnqueue&&,
		OnComplete&&
	) -> ImageTransferJob<OnEnqueue, OnComplete>;

	template<
		std::invocable OnEnqueue,
		std::invocable OnComplete
	>
	ImageBlitJob(
		Image&,
		OnEnqueue&&,
		OnComplete&&
	) -> ImageBlitJob<OnEnqueue, OnComplete>;
}

#endif // !MARCHING_CUBES_CORE_JOBS_HPP

