"""Create linker-only import descriptions for public iOS system frameworks."""
import json
import sys
from pathlib import Path

frameworks = {
    "Foundation": ("NSArray NSAttributedString NSData NSDateFormatter NSDictionary NSLayoutConstraint NSLocale NSMutableArray NSBundle NSJSONSerialization NSNumber NSObject NSString NSTimeZone NSUserDefaults",
                   "_NSStringFromClass _NSClassFromString"),
    "CoreFoundation": ("", "___CFConstantStringClassReference"),
    "CoreGraphics": ("", "_CGRectZero"),
    "UIKit": ("UIAlertAction UIAlertController UIBlurEffect UIButton UIButtonConfiguration UIColor UIDatePicker UIDocumentPickerViewController UIFont UIImage UILabel UIResponder UIScreen UISearchBar UISegmentedControl UISheetPresentationControllerDetent UISwipeGestureRecognizer UIStackView UITableView UITableViewCell UITextField UIView UIViewController UIVisualEffectView UIWindow UIWindowScene UIAction UIMenu",
              "_UIApplicationMain _UIFontTextStyleBody _UIFontTextStyleHeadline _UIFontTextStyleTitle2 _NSForegroundColorAttributeName"),
    "UniformTypeIdentifiers": ("", "_UTTypeJSON"),
    "WebKit": ("WKContentWorld WKUserScript WKWebView WKWebViewConfiguration WKWebsiteDataStore", ""),
}
libraries = {
    **{f"/System/Library/Frameworks/{name}.framework/{name}": exports
       for name, exports in frameworks.items()},
    "/usr/lib/libobjc.A.dylib": ("", """_objc_release _objc_msgSend _objc_retainAutoreleasedReturnValue
        _objc_alloc _objc_unsafeClaimAutoreleasedReturnValue _objc_retain _objc_storeStrong
        _objc_autoreleasePoolPop _objc_autoreleasePoolPush __objc_empty_cache
        _objc_autoreleaseReturnValue _objc_retainAutoreleaseReturnValue _objc_destroyWeak _objc_copyWeak
        _objc_initWeak _objc_storeWeak _objc_loadWeakRetained _objc_setProperty_nonatomic_copy ___objc_personality_v0"""),
    "/usr/lib/libSystem.B.dylib": ("", "__NSConcreteStackBlock __NSConcreteGlobalBlock __Block_object_assign ___stack_chk_fail ___stack_chk_guard __Unwind_Resume"),
}
directory = Path(sys.argv[1])
directory.mkdir(parents=True, exist_ok=True)
for install_name, (classes, symbols) in libraries.items():
    name = install_name.rsplit("/", 1)[-1]
    (directory / f"{name}.tbd").write_text(
        "--- !tapi-tbd\n"
        "tbd-version: 4\n"
        "targets: [ arm64-ios ]\n"
        f"install-name: {json.dumps(install_name)}\n"
        "exports:\n"
        "  - targets: [ arm64-ios ]\n"
        f"    symbols: {json.dumps(symbols.split())}\n"
        f"    objc-classes: {json.dumps(classes.split())}\n"
        "...\n"
    )
