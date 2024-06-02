import MetalKit
import UIKit

@objc class MetalView: UIView {
  override class var layerClass: AnyClass {
    CAMetalLayer.self
  }

  @objc var metalLayer: CAMetalLayer {
    layer as! CAMetalLayer
  }
}
