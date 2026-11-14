#if __has_include(<UIKit/UIKit.h>)
#import <UIKit/UIKit.h>
#import <WebKit/WebKit.h>
#else
#import "../Build/AppleInterfaces.h"
#endif

#include "LedgerInterface.inc"

- (void)loadLedgerInWebView:(WKWebView *)web {
    NSString *saved = [[NSUserDefaults standardUserDefaults] stringForKey:@"ledgerJSON"];
#if FLOWLEDGER_SIMULATOR_PREVIEW
    if (!saved) {
        NSString *previewPath = [[NSBundle mainBundle] pathForResource:@"PreviewLedger" ofType:@"json"];
        saved = [NSString stringWithContentsOfFile:previewPath encoding:NSUTF8StringEncoding error:NULL];
    }
#endif
    // JSON is injected as one expression in a user script, never as HTML.
    NSString *script = saved ? [@"window.initialLedger = " stringByAppendingString:saved] : @"window.initialLedger = []";
    WKUserContentController *content = web.configuration.userContentController;
    [content removeAllUserScripts];
    [content removeScriptMessageHandlerForName:@"ledger" contentWorld:[WKContentWorld pageWorld]];
    [content removeScriptMessageHandlerForName:@"ledgerUI" contentWorld:[WKContentWorld pageWorld]];
    [content addScriptMessageHandlerWithReply:self contentWorld:[WKContentWorld pageWorld] name:@"ledgerUI"];
    [content addScriptMessageHandlerWithReply:self contentWorld:[WKContentWorld pageWorld] name:@"ledger"];
    WKUserScript *bootstrap = [[WKUserScript alloc] initWithSource:script injectionTime:WKUserScriptInjectionTimeAtDocumentStart forMainFrameOnly:YES];
    [content addUserScript:bootstrap];
    NSURL *folder = [[NSBundle mainBundle] URLForResource:@"Web" withExtension:nil];
    [web loadFileURL:[folder URLByAppendingPathComponent:@"index.html"] allowingReadAccessToURL:folder];
}

- (void)userContentController:(WKUserContentController *)controller
      didReceiveScriptMessage:(WKScriptMessage *)message
                 replyHandler:(void (^)(id, NSString *))reply {
    if (message.frameInfo.isMainFrame && [message.name isEqualToString:@"ledgerUI"]
        && [message.body isKindOfClass:[NSDictionary class]]) {
        [self applyPageState:message.body];
        reply(@YES, nil);
        return;
    }
    if (!message.frameInfo.isMainFrame || ![message.body isKindOfClass:[NSString class]]) {
        reply(nil, @"Invalid ledger message");
        return;
    }
    NSData *data = [message.body dataUsingEncoding:NSUTF8StringEncoding];
    id ledger = [NSJSONSerialization JSONObjectWithData:data options:0 error:NULL];
    if (![ledger isKindOfClass:[NSArray class]]) {
        reply(nil, @"Invalid ledger data");
        return;
    }
    // Store Foundation-serialized JSON so injected content is always valid JSON.
    NSData *clean = [NSJSONSerialization dataWithJSONObject:ledger options:0 error:NULL];
    if (!clean) {
        reply(nil, @"Could not encode ledger");
        return;
    }
    NSString *saved = [[NSString alloc] initWithData:clean encoding:NSUTF8StringEncoding];
    [[NSUserDefaults standardUserDefaults] setObject:saved forKey:@"ledgerJSON"];
    reply(@YES, nil);
}

- (void)webViewWebContentProcessDidTerminate:(WKWebView *)webView {
    self.addButton.enabled = NO;
    [self loadLedgerInWebView:webView];
}

- (void)webView:(WKWebView *)webView
    runJavaScriptConfirmPanelWithMessage:(NSString *)message
    initiatedByFrame:(WKFrameInfo *)frame
    completionHandler:(void (^)(BOOL))complete {
    UIAlertController *alert = [UIAlertController alertControllerWithTitle:@"기록 삭제"
        message:message preferredStyle:UIAlertControllerStyleAlert];
    [alert addAction:[UIAlertAction actionWithTitle:@"취소" style:UIAlertActionStyleCancel
        handler:^(UIAlertAction *action) { complete(NO); }]];
    [alert addAction:[UIAlertAction actionWithTitle:@"삭제" style:UIAlertActionStyleDestructive
        handler:^(UIAlertAction *action) { complete(YES); }]];
    [self presentViewController:alert animated:YES completion:nil];
}
@end

@interface LedgerAppDelegate : UIResponder <UIApplicationDelegate>
@end

@implementation LedgerAppDelegate
@end

@interface LedgerSceneDelegate : UIResponder <UIWindowSceneDelegate>
@property(strong, nonatomic) UIWindow *window;
@end

@implementation LedgerSceneDelegate
- (void)scene:(UIScene *)scene
    willConnectToSession:(UISceneSession *)session
    options:(UISceneConnectionOptions *)options {
    if (![scene isKindOfClass:[UIWindowScene class]]) return;
    self.window = [[UIWindow alloc] initWithWindowScene:(UIWindowScene *)scene];
    self.window.rootViewController = [LedgerController new];
    [self.window makeKeyAndVisible];
}
@end

int main(int argc, char *argv[]) {
    @autoreleasepool {
        return UIApplicationMain(argc, argv, nil, NSStringFromClass([LedgerAppDelegate class]));
    }
}
