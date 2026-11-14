// Public Objective-C ABI subset used for the optional SDK-free cross-build.
// An Xcode build uses UIKit and WebKit headers from the selected iPhoneOS SDK.
#define nil ((id)0)
#define NULL ((void *)0)
#define YES __objc_yes
#define NO __objc_no

typedef _Bool BOOL;
typedef unsigned long NSUInteger;
typedef long NSInteger;
typedef double CGFloat;
typedef NSUInteger NSStringEncoding;
enum { NSUTF8StringEncoding = 4 };

typedef struct CGPoint { CGFloat x, y; } CGPoint;
typedef struct CGSize { CGFloat width, height; } CGSize;
typedef struct CGRect { CGPoint origin; CGSize size; } CGRect;

extern const CGRect CGRectZero;

@interface NSObject
+ (instancetype)new;
+ (instancetype)alloc;
+ (Class)class;
- (BOOL)isKindOfClass:(Class)type;
@end

@class NSData, NSError, NSURL, NSString, NSDictionary, NSArray;
@interface NSString : NSObject
- (instancetype)initWithData:(NSData *)data encoding:(NSStringEncoding)encoding;
- (NSString *)stringByAppendingString:(NSString *)text;
- (NSData *)dataUsingEncoding:(NSStringEncoding)encoding;
@end
@interface NSNumber : NSObject
+ (NSNumber *)numberWithBool:(BOOL)value;
@end
@interface NSArray<__covariant ObjectType> : NSObject
@end
@interface NSBundle : NSObject
+ (NSBundle *)mainBundle;
- (NSURL *)URLForResource:(NSString *)name withExtension:(NSString *)ext;
@end
@interface NSURL : NSObject
- (NSURL *)URLByAppendingPathComponent:(NSString *)path;
@end
@interface NSUserDefaults : NSObject
+ (NSUserDefaults *)standardUserDefaults;
- (NSString *)stringForKey:(NSString *)key;
- (void)setObject:(id)value forKey:(NSString *)key;
@end
@interface NSJSONSerialization : NSObject
+ (id)JSONObjectWithData:(NSData *)data options:(NSUInteger)options error:(NSError **)error;
+ (NSData *)dataWithJSONObject:(id)object options:(NSUInteger)options error:(NSError **)error;
@end
extern NSString *NSStringFromClass(Class type);

@class UIApplication, UIViewController, UIAlertAction, UIColor;
@interface UIResponder : NSObject
@end
@interface UIView : UIResponder
@property(nonatomic, getter=isOpaque) BOOL opaque;
@property(nonatomic, strong) UIColor *backgroundColor;
@end
@class UISceneSession, UISceneConnectionOptions;
@interface UIScene : UIResponder
@end
@interface UIWindowScene : UIScene
@end
@protocol UIWindowSceneDelegate
@end
@interface UIWindow : UIView
- (instancetype)initWithWindowScene:(UIWindowScene *)scene;
@property(nonatomic, strong) UIViewController *rootViewController;
- (void)makeKeyAndVisible;
@end
@interface UIScreen : NSObject
+ (UIScreen *)mainScreen;
@property(nonatomic, readonly) CGRect bounds;
@end
@interface UIScrollView : UIView
@property(nonatomic) NSInteger contentInsetAdjustmentBehavior;
@property(nonatomic) NSInteger keyboardDismissMode;
@end
enum { UIScrollViewContentInsetAdjustmentNever = 2, UIScrollViewKeyboardDismissModeInteractive = 2 };
@interface UIColor : NSObject
+ (UIColor *)colorWithRed:(CGFloat)red green:(CGFloat)green blue:(CGFloat)blue alpha:(CGFloat)alpha;
@end
@interface UIViewController : UIResponder
@property(nonatomic, strong) UIView *view;
- (void)presentViewController:(UIViewController *)controller animated:(BOOL)animated completion:(void (^)(void))completion;
@end
enum { UIAlertControllerStyleAlert = 1, UIAlertActionStyleCancel = 1, UIAlertActionStyleDestructive = 2 };
@interface UIAlertAction : NSObject
+ (instancetype)actionWithTitle:(NSString *)title style:(NSInteger)style handler:(void (^)(UIAlertAction *))handler;
@end
@interface UIAlertController : UIViewController
+ (instancetype)alertControllerWithTitle:(NSString *)title message:(NSString *)message preferredStyle:(NSInteger)style;
- (void)addAction:(UIAlertAction *)action;
@end
@protocol UIApplicationDelegate
@end
extern int UIApplicationMain(int argc, char **argv, NSString *principalClassName, NSString *delegateClassName);

@class WKWebView, WKWebViewConfiguration, WKUserContentController, WKScriptMessage;
@class WKWebsiteDataStore, WKContentWorld, WKUserScript, WKFrameInfo, WKNavigation;
@protocol WKUIDelegate
@end
@protocol WKNavigationDelegate
@end
@protocol WKScriptMessageHandlerWithReply
- (void)userContentController:(WKUserContentController *)controller didReceiveScriptMessage:(WKScriptMessage *)message replyHandler:(void (^)(id, NSString *))handler;
@end
@interface WKWebView : UIView
- (instancetype)initWithFrame:(CGRect)frame configuration:(WKWebViewConfiguration *)configuration;
@property(nonatomic, readonly) WKWebViewConfiguration *configuration;
@property(nonatomic, readonly) UIScrollView *scrollView;
@property(nonatomic, weak) id <WKUIDelegate> UIDelegate;
@property(nonatomic, weak) id <WKNavigationDelegate> navigationDelegate;
- (WKNavigation *)loadFileURL:(NSURL *)url allowingReadAccessToURL:(NSURL *)resourceURL;
@end
@interface WKWebViewConfiguration : NSObject
@property(nonatomic, strong) WKWebsiteDataStore *websiteDataStore;
@property(nonatomic, strong) WKUserContentController *userContentController;
@end
@interface WKWebsiteDataStore : NSObject
+ (WKWebsiteDataStore *)nonPersistentDataStore;
@end
@interface WKContentWorld : NSObject
+ (WKContentWorld *)pageWorld;
@end
@interface WKUserContentController : NSObject
- (void)addUserScript:(WKUserScript *)script;
- (void)removeAllUserScripts;
- (void)addScriptMessageHandlerWithReply:(id <WKScriptMessageHandlerWithReply>)handler contentWorld:(WKContentWorld *)world name:(NSString *)name;
- (void)removeScriptMessageHandlerForName:(NSString *)name contentWorld:(WKContentWorld *)world;
@end
enum { WKUserScriptInjectionTimeAtDocumentStart = 0 };
@interface WKUserScript : NSObject
- (instancetype)initWithSource:(NSString *)source injectionTime:(NSInteger)injectionTime forMainFrameOnly:(BOOL)mainFrameOnly;
@end
@interface WKFrameInfo : NSObject
@property(nonatomic, readonly, getter=isMainFrame) BOOL mainFrame;
@end
@interface WKScriptMessage : NSObject
@property(nonatomic, readonly) id body;
@property(nonatomic, readonly) WKFrameInfo *frameInfo;
@end

@class NSLayoutConstraint, NSLayoutAnchor, NSLayoutXAxisAnchor, NSLayoutYAxisAnchor, NSLayoutDimension;
@class UILayoutGuide, UIKeyboardLayoutGuide, UIFont, UIImage, UIButtonConfiguration, UIMenu, UIAction, NSDate;

@interface NSArray (Literals)
+ (instancetype)arrayWithObjects:(const id [])objects count:(NSUInteger)count;
- (id)objectAtIndexedSubscript:(NSUInteger)index;
@property(readonly) NSUInteger count;
@end
@interface NSDictionary<__covariant KeyType, __covariant ObjectType> : NSObject
+ (instancetype)dictionaryWithObjects:(const id [])objects forKeys:(const id [])keys count:(NSUInteger)count;
- (ObjectType)objectForKeyedSubscript:(KeyType)key;
@end
@interface NSMutableArray<ObjectType> : NSArray<ObjectType>
- (void)addObject:(ObjectType)object;
@end
@interface NSString (Display)
- (BOOL)isEqualToString:(NSString *)text;
@end
@interface NSNumber (Value)
@property(readonly) BOOL boolValue;
@end
@interface NSObject (Selectors)
+ (BOOL)respondsToSelector:(SEL)selector;
@end
extern Class NSClassFromString(NSString *name);

@interface NSLayoutAnchor<AnchorType> : NSObject
- (NSLayoutConstraint *)constraintEqualToAnchor:(AnchorType)anchor;
- (NSLayoutConstraint *)constraintEqualToAnchor:(AnchorType)anchor constant:(CGFloat)constant;
@end
@interface NSLayoutXAxisAnchor : NSLayoutAnchor<NSLayoutXAxisAnchor *> @end
@interface NSLayoutYAxisAnchor : NSLayoutAnchor<NSLayoutYAxisAnchor *> @end
@interface NSLayoutDimension : NSLayoutAnchor<NSLayoutDimension *>
- (NSLayoutConstraint *)constraintEqualToConstant:(CGFloat)constant;
- (NSLayoutConstraint *)constraintGreaterThanOrEqualToConstant:(CGFloat)constant;
@end
@interface NSLayoutConstraint : NSObject
+ (void)activateConstraints:(NSArray<NSLayoutConstraint *> *)constraints;
@property BOOL active;
@end
@interface UILayoutGuide : NSObject
@property(readonly) NSLayoutYAxisAnchor *topAnchor;
@property(readonly) NSLayoutYAxisAnchor *bottomAnchor;
@property(readonly) NSLayoutXAxisAnchor *leadingAnchor;
@property(readonly) NSLayoutXAxisAnchor *trailingAnchor;
@end
@interface UIKeyboardLayoutGuide : UILayoutGuide @end

@interface UIView (Layout)
- (instancetype)initWithFrame:(CGRect)frame;
- (void)addSubview:(UIView *)view;
- (BOOL)endEditing:(BOOL)force;
@property BOOL translatesAutoresizingMaskIntoConstraints;
@property(getter=isHidden) BOOL hidden;
@property(strong) NSString *accessibilityLabel;
@property(strong) UIColor *tintColor;
@property(readonly) UILayoutGuide *safeAreaLayoutGuide;
@property(readonly) UIKeyboardLayoutGuide *keyboardLayoutGuide;
@property(readonly) NSLayoutYAxisAnchor *topAnchor;
@property(readonly) NSLayoutYAxisAnchor *bottomAnchor;
@property(readonly) NSLayoutXAxisAnchor *leadingAnchor;
@property(readonly) NSLayoutXAxisAnchor *trailingAnchor;
@property(readonly) NSLayoutDimension *heightAnchor;
@property(readonly) NSLayoutDimension *widthAnchor;
@end

@interface UIViewController (Lifecycle)
- (void)loadView;
@end
@interface UIColor (Semantic)
+ (UIColor *)systemBackgroundColor;
+ (UIColor *)secondarySystemBackgroundColor;
+ (UIColor *)labelColor;
+ (UIColor *)systemIndigoColor;
@end
@interface UIFont : NSObject
+ (UIFont *)preferredFontForTextStyle:(NSString *)style;
@end
extern NSString * const UIFontTextStyleHeadline;
extern NSString * const UIFontTextStyleBody;
@interface UIImage : NSObject
+ (UIImage *)systemImageNamed:(NSString *)name;
@end
@interface UILabel : UIView
@property(strong) NSString *text;
@property(strong) UIFont *font;
@property NSInteger textAlignment;
@property BOOL adjustsFontForContentSizeCategory;
@property BOOL adjustsFontSizeToFitWidth;
@end
enum { NSTextAlignmentCenter = 1 };
@interface UIStackView : UIView
- (instancetype)initWithArrangedSubviews:(NSArray<UIView *> *)views;
@property NSInteger axis;
@property NSInteger distribution;
@property NSInteger alignment;
@property CGFloat spacing;
@end
enum { UILayoutConstraintAxisVertical = 1, UIStackViewDistributionFillEqually = 1, UIStackViewAlignmentCenter = 3 };

@interface UIControl : UIView
- (void)addTarget:(id)target action:(SEL)action forControlEvents:(NSUInteger)events;
@property(getter=isEnabled) BOOL enabled;
@end
enum { UIControlEventValueChanged = 1 << 12, UIControlEventTouchUpInside = 1 << 6 };
@interface UISegmentedControl : UIControl
- (instancetype)initWithItems:(NSArray *)items;
@property NSInteger selectedSegmentIndex;
@end
@interface UIButtonConfiguration : NSObject
+ (instancetype)plainButtonConfiguration;
+ (instancetype)tintedButtonConfiguration;
+ (instancetype)filledButtonConfiguration;
+ (instancetype)glassButtonConfiguration;
+ (instancetype)prominentGlassButtonConfiguration;
@property(strong) NSString *title;
@property(strong) UIImage *image;
@end
@interface UIButton : UIControl
+ (instancetype)buttonWithConfiguration:(UIButtonConfiguration *)configuration primaryAction:(UIAction *)action;
@property(strong) UIButtonConfiguration *configuration;
@property(strong) UIMenu *menu;
@property BOOL showsMenuAsPrimaryAction;
@end
@interface UIAction : NSObject
+ (instancetype)actionWithTitle:(NSString *)title image:(UIImage *)image identifier:(NSString *)identifier handler:(void (^)(UIAction *))handler;
@property NSInteger state;
@end
enum { UIMenuElementStateOn = 1 };
@interface UIMenu : NSObject
+ (instancetype)menuWithTitle:(NSString *)title children:(NSArray *)children;
@end

@class UITextField;
@protocol UITextFieldDelegate
- (BOOL)textFieldShouldReturn:(UITextField *)field;
@end
@interface UITextField : UIControl
@property(strong) NSString *placeholder;
@property(strong) NSString *text;
@property(strong) UIFont *font;
@property BOOL adjustsFontForContentSizeCategory;
@property NSInteger borderStyle;
@property NSInteger keyboardType;
@property NSInteger returnKeyType;
@property NSInteger clearButtonMode;
@property(weak) id<UITextFieldDelegate> delegate;
- (BOOL)becomeFirstResponder;
@end
enum { UITextBorderStyleRoundedRect = 3, UIKeyboardTypeNumberPad = 4, UIReturnKeyNext = 4, UITextFieldViewModeWhileEditing = 1 };

@interface NSDate : NSObject @end
@interface UIDatePicker : UIControl
@property NSInteger datePickerMode;
@property NSInteger preferredDatePickerStyle;
@property(strong) NSDate *date;
@property(strong) NSDate *minimumDate;
@property(strong) NSDate *maximumDate;
@end
enum { UIDatePickerModeDate = 1, UIDatePickerStyleCompact = 2 };
@interface NSDateFormatter : NSObject
@property(strong) NSString *dateFormat;
@property(strong) id locale;
@property(strong) id timeZone;
- (NSDate *)dateFromString:(NSString *)string;
- (NSString *)stringFromDate:(NSDate *)date;
@end
@interface NSLocale : NSObject
- (instancetype)initWithLocaleIdentifier:(NSString *)identifier;
@end
@interface NSTimeZone : NSObject
+ (instancetype)localTimeZone;
@end

@interface UIVisualEffect : NSObject @end
@interface UIBlurEffect : UIVisualEffect
+ (instancetype)effectWithStyle:(NSInteger)style;
@end
enum { UIBlurEffectStyleSystemChromeMaterial = 10 };
@interface UIVisualEffectView : UIView
- (instancetype)initWithEffect:(UIVisualEffect *)effect;
@property(readonly) UIView *contentView;
@end

@interface WKWebView (ScriptCalls)
@property(strong) UIColor *underPageBackgroundColor;
- (void)callAsyncJavaScript:(NSString *)body arguments:(NSDictionary *)arguments inFrame:(WKFrameInfo *)frame inContentWorld:(WKContentWorld *)world completionHandler:(void (^)(id, NSError *))handler;
@end
@interface UIView (Hierarchy)
@property(readonly) UIView *superview;
@end
@interface UIColor (Grouped)
+ (UIColor *)systemGroupedBackgroundColor;
@end
@interface NSLayoutDimension (Proportions)
- (NSLayoutConstraint *)constraintEqualToAnchor:(NSLayoutDimension *)anchor multiplier:(CGFloat)multiplier;
@end
@interface WKScriptMessage (Name)
@property(readonly) NSString *name;
@end
extern NSString * const UIFontTextStyleTitle2;
@interface NSNumber (Boxing)
+ (NSNumber *)numberWithInt:(int)value;
@end

@interface NSObject (InstanceSelectors)
- (BOOL)respondsToSelector:(SEL)selector;
@end

@interface CALayer : NSObject
@property CGFloat cornerRadius;
@end

@class UIGestureRecognizer;
@interface UIView (PanelAppearance)
@property BOOL clipsToBounds;
@property BOOL userInteractionEnabled;
@property BOOL isAccessibilityElement;
@property(readonly) CALayer *layer;
@property(readonly) NSLayoutXAxisAnchor *centerXAnchor;
@property(readonly) NSLayoutYAxisAnchor *centerYAnchor;
- (void)addGestureRecognizer:(UIGestureRecognizer *)gesture;
@end

@interface UIColor (PanelColors)
+ (UIColor *)tertiaryLabelColor;
+ (UIColor *)systemBlueColor;
- (UIColor *)colorWithAlphaComponent:(CGFloat)alpha;
@end

@interface UIButtonConfiguration (ActionAppearance)
@property CGFloat imagePadding;
@property(strong) UIColor *baseForegroundColor;
@end

@interface UIGestureRecognizer : NSObject
- (instancetype)initWithTarget:(id)target action:(SEL)action;
@end
@interface UISwipeGestureRecognizer : UIGestureRecognizer
@property NSUInteger direction;
@end
enum { UISwipeGestureRecognizerDirectionUp = 1 << 2, UISwipeGestureRecognizerDirectionDown = 1 << 3 };

@interface UIButtonConfiguration (Summary)
@property(copy) NSString *subtitle;
@property NSInteger titleAlignment;
@end
enum { UIButtonConfigurationTitleAlignmentCenter = 2 };
@interface UIColor (Balance)
+ (UIColor *)systemRedColor;
@end

@interface UIViewController (RecordEditor)
@property(readonly) UIViewController *presentedViewController;
@property NSInteger modalPresentationStyle;
@property(getter=isModalInPresentation) BOOL modalInPresentation;
- (void)dismissViewControllerAnimated:(BOOL)animated completion:(void (^)(void))completion;
@end
enum { UIModalPresentationFormSheet = 2 };
@interface UILabel (RecordEditor)
@property NSInteger numberOfLines;
@end

@interface NSString (InputLength)
@property(readonly) NSUInteger length;
@end
@interface UIView (AccessibleValue)
@property(copy) NSString *accessibilityValue;
@end
@interface NSLayoutConstraint (LayoutPriority)
@property float priority;
@end
@interface UITextField (InputInsets)
@property(strong) UIView *leftView;
@property NSInteger leftViewMode;
@property(readonly, getter=isFirstResponder) BOOL firstResponder;
@end
@interface UIButtonConfiguration (Truncation)
@property NSInteger titleLineBreakMode;
@end
enum {
    UITextBorderStyleNone = 0, UITextFieldViewModeAlways = 3,
    NSLineBreakByTruncatingTail = 4, UIDatePickerStyleInline = 3,
    UIControlEventEditingDidBegin = 1 << 16, UIControlEventEditingChanged = 1 << 17,
    UIControlEventEditingDidEnd = 1 << 18
};

@class UISheetPresentationController, NSIndexPath, UITableView, UITableViewCell;
@interface UISheetPresentationController : NSObject
@property BOOL prefersGrabberVisible;
@end
@interface UIViewController (DetailSheet)
@property(readonly) UISheetPresentationController *sheetPresentationController;
@end
@interface NSString (DetailFormat)
+ (instancetype)stringWithFormat:(NSString *)format, ...;
@end
@interface NSIndexPath : NSObject
@property(readonly) NSInteger row;
@end
typedef NSInteger UITableViewCellEditingStyle;
enum { UITableViewStyleInsetGrouped = 2, UITableViewCellStyleSubtitle = 3,
       UITableViewCellAccessoryDisclosureIndicator = 1, UITableViewCellEditingStyleDelete = 1 };
@protocol UITableViewDataSource
- (NSInteger)tableView:(UITableView *)table numberOfRowsInSection:(NSInteger)section;
- (UITableViewCell *)tableView:(UITableView *)table cellForRowAtIndexPath:(NSIndexPath *)indexPath;
@end
@protocol UITableViewDelegate
@end
@interface UITableView : UIScrollView
- (instancetype)initWithFrame:(CGRect)frame style:(NSInteger)style;
@property(weak) id<UITableViewDataSource> dataSource;
@property(weak) id<UITableViewDelegate> delegate;
- (UITableViewCell *)dequeueReusableCellWithIdentifier:(NSString *)identifier;
- (void)reloadData;
- (void)deselectRowAtIndexPath:(NSIndexPath *)path animated:(BOOL)animated;
@end
@interface UITableViewCell : UIView
- (instancetype)initWithStyle:(NSInteger)style reuseIdentifier:(NSString *)identifier;
@property(readonly) UILabel *textLabel;
@property(readonly) UILabel *detailTextLabel;
@property NSInteger accessoryType;
@end
