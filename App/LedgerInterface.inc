// Native controls around the shared Sankey and ledger view.
#if !__has_include(<UIKit/UIKit.h>) || __IPHONE_OS_VERSION_MAX_ALLOWED < 260000
@interface UIButtonConfiguration (LedgerGlass)
+ (instancetype)glassButtonConfiguration;
+ (instancetype)prominentGlassButtonConfiguration;
@end
#endif

// Resolve iOS 26-only classes at runtime so the same binary still launches on iOS 16.4.
@protocol LedgerGlassEffect
+ (instancetype)effectWithStyle:(NSInteger)style;
@end

@protocol LedgerGlassCorners
+ (id)fixedRadius:(CGFloat)radius;
+ (id)configurationWithUniformRadius:(id)radius;
- (void)setCornerConfiguration:(id)configuration;
@end

@interface LedgerController : UIViewController <WKUIDelegate, WKNavigationDelegate, WKScriptMessageHandlerWithReply, UITextFieldDelegate, UITableViewDataSource, UITableViewDelegate>
@property(nonatomic, strong) WKWebView *web;
@property(nonatomic, strong) UIStackView *entryBody;
@property(nonatomic, strong) UIButton *entryHandle;
@property(nonatomic, strong) UILabel *collapsedTitle;
@property(nonatomic, strong) NSLayoutConstraint *expandedBottom;
@property(nonatomic, strong) NSLayoutConstraint *collapsedBottom;
@property(nonatomic) BOOL entryExpanded;
@property(nonatomic, strong) UILabel *period;
@property(nonatomic, strong) NSArray<UIButton *> *summaryButtons;
@property(nonatomic, strong) UISegmentedControl *periodUnit;
@property(nonatomic, strong) UISegmentedControl *entryKind;
@property(nonatomic, strong) UIButton *previousButton;
@property(nonatomic, strong) UIButton *nextButton;
@property(nonatomic, strong) UIButton *categoryButton;
@property(nonatomic, strong) UIButton *addButton;
@property(nonatomic, strong) UITextField *nameField;
@property(nonatomic, strong) UITextField *amountField;
@property(nonatomic, strong) NSDate *entryDate;
@property(nonatomic, strong) UIButton *entryDateButton;
@property(nonatomic, strong) UIStackView *nameSuggestions;
@property(nonatomic, strong) NSArray<UIButton *> *suggestionButtons;
@property(nonatomic) NSInteger suggestionRequest;
@property(nonatomic, strong) UIDatePicker *calendarPicker;
@property(nonatomic) BOOL editingCalendar;
@property(nonatomic, strong) NSDateFormatter *dateFormat;
@property(nonatomic, strong) NSDictionary *categories;
@property(nonatomic, copy) NSString *categoryID;
@property(nonatomic, copy) NSString *categoryLabel;
@property(nonatomic, copy) NSString *defaultEntryDate;
@property(nonatomic) BOOL selectedCustomDate;
@property(nonatomic) BOOL busy;
@property(nonatomic) BOOL ledgerAvailable;
@property(nonatomic, weak) UIViewController *detailSheet;
@property(nonatomic, weak) UITableView *detailTable;
@property(nonatomic, weak) UILabel *detailTitle;
@property(nonatomic, weak) UILabel *detailPeriod;
@property(nonatomic, weak) UILabel *detailTotal;
@property(nonatomic, weak) UILabel *detailExplanation;
@property(nonatomic, strong) NSArray *detailEntries;
@property(nonatomic, strong) UIViewController *recordEditor;
@property(nonatomic, strong) UITextField *editNameField;
@property(nonatomic, strong) UITextField *editAmountField;
@property(nonatomic, strong) NSDate *editDate;
@property(nonatomic, strong) UIButton *editDateButton;
@property(nonatomic, strong) UIButton *editCategoryButton;
@property(nonatomic, strong) UIButton *saveEditButton;
@property(nonatomic, strong) UIButton *cancelEditButton;
@property(nonatomic, strong) UILabel *editError;
@property(nonatomic, copy) NSString *editID;
@property(nonatomic, copy) NSString *editKind;
@property(nonatomic, copy) NSString *editCategory;
@end

@implementation LedgerController

- (void)loadView {
    UIView *root = [UIView new];
    root.backgroundColor = [UIColor systemGroupedBackgroundColor];
    root.tintColor = [UIColor labelColor];
    self.view = root;

    WKWebViewConfiguration *configuration = [WKWebViewConfiguration new];
    configuration.websiteDataStore = [WKWebsiteDataStore nonPersistentDataStore];
    self.web = [[WKWebView alloc] initWithFrame:CGRectZero configuration:configuration];
    self.web.UIDelegate = self;
    self.web.navigationDelegate = self;
    self.web.scrollView.contentInsetAdjustmentBehavior = UIScrollViewContentInsetAdjustmentNever;
    self.web.scrollView.keyboardDismissMode = UIScrollViewKeyboardDismissModeInteractive;
    self.web.opaque = NO;
    self.web.backgroundColor = [UIColor systemGroupedBackgroundColor];
    self.web.underPageBackgroundColor = [UIColor systemGroupedBackgroundColor];
    self.web.translatesAutoresizingMaskIntoConstraints = NO;

    self.periodUnit = [[UISegmentedControl alloc] initWithItems:@[@"월별", @"연별"]];
    self.periodUnit.accessibilityLabel = @"집계 단위";
    self.periodUnit.selectedSegmentIndex = 0;
    [self.periodUnit addTarget:self action:@selector(changePeriodUnit) forControlEvents:UIControlEventValueChanged];

    self.period = [UILabel new];
    self.period.font = [UIFont preferredFontForTextStyle:UIFontTextStyleTitle2];
    self.period.textAlignment = NSTextAlignmentCenter;
    self.period.adjustsFontForContentSizeCategory = YES;
    self.period.adjustsFontSizeToFitWidth = YES;
    self.previousButton = [self buttonWithTitle:nil icon:@"chevron.left" primary:NO action:@selector(previousPeriod)];
    self.nextButton = [self buttonWithTitle:nil icon:@"chevron.right" primary:NO action:@selector(nextPeriod)];
    self.previousButton.accessibilityLabel = @"이전 기간";
    self.nextButton.accessibilityLabel = @"다음 기간";
    self.previousButton.enabled = NO;
    self.nextButton.enabled = NO;
    UIStackView *periodRow = [self row:@[self.previousButton, self.period, self.nextButton] spacing:8];
    self.summaryButtons = @[
        [self actionButton:@"수입" icon:nil action:@selector(showIncome)],
        [self actionButton:@"지출" icon:nil action:@selector(showExpense)],
        [self actionButton:@"남은 금액" icon:nil action:@selector(showBalance)],
    ];
    UIStackView *summaryRow = [self row:self.summaryButtons spacing:4];
    summaryRow.distribution = UIStackViewDistributionFillEqually;
    summaryRow.translatesAutoresizingMaskIntoConstraints = NO;
    UIVisualEffectView *summary = [self glassSurfaceWithRadius:24];
    [summary.contentView addSubview:summaryRow];
    UIStackView *header = [[UIStackView alloc] initWithArrangedSubviews:@[self.periodUnit, periodRow, summary]];
    header.axis = UILayoutConstraintAxisVertical;
    header.spacing = 8;
    header.translatesAutoresizingMaskIntoConstraints = NO;

    self.entryKind = [[UISegmentedControl alloc] initWithItems:@[@"지출", @"수입"]];
    self.entryKind.selectedSegmentIndex = 0;
    self.entryKind.accessibilityLabel = @"거래 유형";
    [self.entryKind addTarget:self action:@selector(changeEntryKind) forControlEvents:UIControlEventValueChanged];

    self.nameField = [self textFieldWithPlaceholder:@"지출 이름"];
    self.nameField.delegate = self;
    self.nameField.returnKeyType = UIReturnKeyNext;
    [self.nameField addTarget:self action:@selector(refreshSuggestions) forControlEvents:UIControlEventEditingChanged];
    [self.nameField addTarget:self action:@selector(refreshSuggestions) forControlEvents:UIControlEventEditingDidBegin];
    [self.nameField addTarget:self action:@selector(hideSuggestions) forControlEvents:UIControlEventEditingDidEnd];
    self.amountField = [self textFieldWithPlaceholder:@"금액 (원)"];
    self.amountField.keyboardType = UIKeyboardTypeNumberPad;
    UIStackView *fields = [self row:@[self.nameField, self.amountField] spacing:8];

    self.categoryButton = [self actionButton:@"카테고리" icon:@"tag" action:NULL];
    self.categoryButton.showsMenuAsPrimaryAction = YES;
    self.categoryButton.enabled = NO;
    self.entryDateButton = [self actionButton:@"날짜" icon:@"calendar" action:@selector(showEntryCalendar)];
    self.entryDateButton.accessibilityLabel = @"기록 날짜";
    self.addButton = [self buttonWithTitle:@"추가" icon:@"plus" primary:YES action:@selector(addEntry)];
    self.addButton.enabled = NO;
    UIStackView *actions = [self row:@[self.categoryButton, self.entryDateButton, self.addButton] spacing:3];
    actions.distribution = UIStackViewDistributionFillEqually;
    actions.translatesAutoresizingMaskIntoConstraints = NO;
    UIVisualEffectView *actionBar = [self glassSurfaceWithRadius:26];
    [actionBar.contentView addSubview:actions];

    self.dateFormat = [NSDateFormatter new];
    self.dateFormat.locale = [[NSLocale alloc] initWithLocaleIdentifier:@"en_US_POSIX"];
    self.dateFormat.timeZone = [NSTimeZone localTimeZone];
    self.dateFormat.dateFormat = @"yyyy-MM-dd";
    self.suggestionButtons = @[
        [self actionButton:nil icon:nil action:@selector(useSuggestion:)],
        [self actionButton:nil icon:nil action:@selector(useSuggestion:)],
        [self actionButton:nil icon:nil action:@selector(useSuggestion:)],
    ];
    self.nameSuggestions = [self row:self.suggestionButtons spacing:4];
    self.nameSuggestions.distribution = UIStackViewDistributionFillEqually;
    self.nameSuggestions.hidden = YES;

    UIVisualEffectView *panel = [self glassSurfaceWithRadius:30];
    UIView *panelContent = panel.contentView;
    self.entryHandle = [self actionButton:nil icon:nil action:@selector(toggleEntryPanel)];
    self.entryHandle.translatesAutoresizingMaskIntoConstraints = NO;
    UIView *grabber = [UIView new];
    grabber.backgroundColor = [UIColor tertiaryLabelColor];
    grabber.layer.cornerRadius = 2;
    grabber.userInteractionEnabled = NO;
    grabber.translatesAutoresizingMaskIntoConstraints = NO;
    [self.entryHandle addSubview:grabber];
    for (NSUInteger direction = UISwipeGestureRecognizerDirectionUp;
         direction <= UISwipeGestureRecognizerDirectionDown; direction <<= 1) {
        UISwipeGestureRecognizer *swipe = [[UISwipeGestureRecognizer alloc] initWithTarget:self action:@selector(swipeEntryPanel:)];
        swipe.direction = direction;
        [self.entryHandle addGestureRecognizer:swipe];
    }

    self.collapsedTitle = [UILabel new];
    self.collapsedTitle.text = @"빠른 추가";
    self.collapsedTitle.font = [UIFont preferredFontForTextStyle:UIFontTextStyleHeadline];
    self.collapsedTitle.textAlignment = NSTextAlignmentCenter;
    self.collapsedTitle.isAccessibilityElement = NO;
    self.collapsedTitle.translatesAutoresizingMaskIntoConstraints = NO;
    [self.entryHandle addSubview:self.collapsedTitle];

    self.entryBody = [[UIStackView alloc] initWithArrangedSubviews:@[self.entryKind, fields, self.nameSuggestions, actionBar]];
    self.entryBody.axis = UILayoutConstraintAxisVertical;
    self.entryBody.spacing = 9;
    self.entryBody.translatesAutoresizingMaskIntoConstraints = NO;
    [panelContent addSubview:self.entryHandle];
    [panelContent addSubview:self.entryBody];
    self.expandedBottom = [self.entryBody.bottomAnchor constraintEqualToAnchor:panel.bottomAnchor constant:-11];
    self.collapsedBottom = [self.entryHandle.bottomAnchor constraintEqualToAnchor:panel.bottomAnchor constant:-5];

    [root addSubview:header];
    [root addSubview:self.web];
    [root addSubview:panel];
    UILayoutGuide *safe = root.safeAreaLayoutGuide;
    NSLayoutConstraint *webBottom = [self.web.bottomAnchor constraintEqualToAnchor:panel.topAnchor constant:12];
    webBottom.priority = 749;
    [NSLayoutConstraint activateConstraints:@[
        [header.topAnchor constraintEqualToAnchor:safe.topAnchor constant:8],
        [header.leadingAnchor constraintEqualToAnchor:safe.leadingAnchor constant:16],
        [header.trailingAnchor constraintEqualToAnchor:safe.trailingAnchor constant:-16],
        [self.periodUnit.heightAnchor constraintGreaterThanOrEqualToConstant:34],
        [self.previousButton.widthAnchor constraintEqualToConstant:44],
        [self.nextButton.widthAnchor constraintEqualToConstant:44],
        [periodRow.heightAnchor constraintGreaterThanOrEqualToConstant:44],
        [summaryRow.heightAnchor constraintGreaterThanOrEqualToConstant:52],
        [summaryRow.leadingAnchor constraintEqualToAnchor:summary.leadingAnchor constant:4],
        [summaryRow.trailingAnchor constraintEqualToAnchor:summary.trailingAnchor constant:-4],
        [summaryRow.topAnchor constraintEqualToAnchor:summary.topAnchor constant:3],
        [summaryRow.bottomAnchor constraintEqualToAnchor:summary.bottomAnchor constant:-3],

        [self.web.topAnchor constraintEqualToAnchor:header.bottomAnchor],
        [self.web.leadingAnchor constraintEqualToAnchor:safe.leadingAnchor],
        [self.web.trailingAnchor constraintEqualToAnchor:safe.trailingAnchor],
        webBottom,
        [self.web.heightAnchor constraintGreaterThanOrEqualToConstant:0],

        [panel.leadingAnchor constraintEqualToAnchor:safe.leadingAnchor constant:9],
        [panel.trailingAnchor constraintEqualToAnchor:safe.trailingAnchor constant:-9],
        [panel.bottomAnchor constraintEqualToAnchor:root.keyboardLayoutGuide.topAnchor constant:-6],
        [self.entryHandle.topAnchor constraintEqualToAnchor:panel.topAnchor],
        [self.entryHandle.leadingAnchor constraintEqualToAnchor:panel.leadingAnchor],
        [self.entryHandle.trailingAnchor constraintEqualToAnchor:panel.trailingAnchor],
        [self.entryHandle.heightAnchor constraintEqualToConstant:44],
        [grabber.topAnchor constraintEqualToAnchor:self.entryHandle.topAnchor constant:8],
        [grabber.centerXAnchor constraintEqualToAnchor:self.entryHandle.centerXAnchor],
        [grabber.widthAnchor constraintEqualToConstant:37],
        [grabber.heightAnchor constraintEqualToConstant:4],
        [self.collapsedTitle.centerXAnchor constraintEqualToAnchor:self.entryHandle.centerXAnchor],
        [self.collapsedTitle.centerYAnchor constraintEqualToAnchor:self.entryHandle.centerYAnchor constant:6],
        [self.entryBody.topAnchor constraintEqualToAnchor:self.entryHandle.bottomAnchor constant:-8],
        [self.entryBody.leadingAnchor constraintEqualToAnchor:panel.leadingAnchor constant:13],
        [self.entryBody.trailingAnchor constraintEqualToAnchor:panel.trailingAnchor constant:-13],
        [self.entryKind.heightAnchor constraintGreaterThanOrEqualToConstant:38],
        [fields.heightAnchor constraintGreaterThanOrEqualToConstant:44],
        [self.amountField.widthAnchor constraintEqualToAnchor:fields.widthAnchor multiplier:0.41],
        [actions.heightAnchor constraintGreaterThanOrEqualToConstant:44],
        [actions.leadingAnchor constraintEqualToAnchor:actionBar.leadingAnchor constant:4],
        [actions.trailingAnchor constraintEqualToAnchor:actionBar.trailingAnchor constant:-4],
        [actions.topAnchor constraintEqualToAnchor:actionBar.topAnchor constant:4],
        [actions.bottomAnchor constraintEqualToAnchor:actionBar.bottomAnchor constant:-4],
    ]];
    [self setEntryPanelExpanded:YES];
    [self loadLedgerInWebView:self.web];
}

- (UIStackView *)row:(NSArray<UIView *> *)views spacing:(CGFloat)spacing {
    UIStackView *row = [[UIStackView alloc] initWithArrangedSubviews:views];
    row.spacing = spacing;
    return row;
}

- (UIVisualEffectView *)glassSurfaceWithRadius:(CGFloat)radius {
    Class<LedgerGlassEffect> glassClass = (Class<LedgerGlassEffect>)NSClassFromString(@"UIGlassEffect");
    Class<LedgerGlassCorners> cornersClass = (Class<LedgerGlassCorners>)NSClassFromString(@"UICornerConfiguration");
    Class<LedgerGlassCorners> radiusClass = (Class<LedgerGlassCorners>)NSClassFromString(@"UICornerRadius");
    BOOL supportsGlass = [(Class)glassClass respondsToSelector:@selector(effectWithStyle:)] &&
        [(Class)cornersClass respondsToSelector:@selector(configurationWithUniformRadius:)] &&
        [(Class)radiusClass respondsToSelector:@selector(fixedRadius:)];
    UIVisualEffect *material = [UIBlurEffect effectWithStyle:UIBlurEffectStyleSystemChromeMaterial];
    if (supportsGlass) {
        UIVisualEffect<LedgerGlassEffect> *glass = [glassClass effectWithStyle:0];
        material = glass;
    }
    UIVisualEffectView *surface = [[UIVisualEffectView alloc] initWithEffect:material];
    surface.translatesAutoresizingMaskIntoConstraints = NO;
    if (supportsGlass && [surface respondsToSelector:@selector(setCornerConfiguration:)]) {
        id corners = [cornersClass configurationWithUniformRadius:[radiusClass fixedRadius:radius]];
        [(id<LedgerGlassCorners>)surface setCornerConfiguration:corners];
    } else {
        surface.clipsToBounds = YES;
        surface.layer.cornerRadius = radius;
    }
    return surface;
}

- (UIButton *)actionButton:(NSString *)title icon:(NSString *)icon action:(SEL)action {
    UIButtonConfiguration *configuration = [UIButtonConfiguration plainButtonConfiguration];
    configuration.title = title;
    if (icon) configuration.image = [UIImage systemImageNamed:icon];
    configuration.imagePadding = 4;
    configuration.baseForegroundColor = [UIColor labelColor];
    UIButton *button = [UIButton buttonWithConfiguration:configuration primaryAction:nil];
    if (action) [button addTarget:self action:action forControlEvents:UIControlEventTouchUpInside];
    return button;
}

- (void)toggleEntryPanel {
    [self setEntryPanelExpanded:!self.entryExpanded];
}

- (void)swipeEntryPanel:(UISwipeGestureRecognizer *)gesture {
    [self setEntryPanelExpanded:gesture.direction == UISwipeGestureRecognizerDirectionUp];
}

- (void)setEntryPanelExpanded:(BOOL)expanded {
    if (!expanded) [self.view endEditing:YES];
    self.entryExpanded = expanded;
    self.entryHandle.accessibilityLabel = expanded ? @"빠른 추가 접기" : @"빠른 추가 펼치기";
    self.collapsedTitle.hidden = expanded;
    self.entryBody.hidden = !expanded;
    // Deactivate both before switching so only one view sizes the panel.
    self.expandedBottom.active = NO;
    self.collapsedBottom.active = NO;
    (expanded ? self.expandedBottom : self.collapsedBottom).active = YES;
}

- (UITextField *)textFieldWithPlaceholder:(NSString *)placeholder {
    UITextField *field = [UITextField new];
    field.placeholder = placeholder;
    field.accessibilityLabel = placeholder;
    field.font = [UIFont preferredFontForTextStyle:UIFontTextStyleBody];
    field.adjustsFontForContentSizeCategory = YES;
    field.borderStyle = UITextBorderStyleNone;
    field.backgroundColor = [[UIColor labelColor] colorWithAlphaComponent:0.08];
    field.layer.cornerRadius = 22;
    field.leftView = [[UIView alloc] initWithFrame:(CGRect){{0, 0}, {14, 1}}];
    field.leftViewMode = UITextFieldViewModeAlways;
    field.clearButtonMode = UITextFieldViewModeWhileEditing;
    return field;
}

- (UIButton *)buttonWithTitle:(NSString *)title icon:(NSString *)icon primary:(BOOL)primary action:(SEL)action {
    UIButtonConfiguration *configuration;
    if ([UIButtonConfiguration respondsToSelector:@selector(glassButtonConfiguration)] &&
        [UIButtonConfiguration respondsToSelector:@selector(prominentGlassButtonConfiguration)]) {
        configuration = primary ? [UIButtonConfiguration prominentGlassButtonConfiguration] : [UIButtonConfiguration glassButtonConfiguration];
    } else {
        configuration = primary ? [UIButtonConfiguration filledButtonConfiguration] : [UIButtonConfiguration tintedButtonConfiguration];
    }
    configuration.title = title;
    if (icon) configuration.image = [UIImage systemImageNamed:icon];
    configuration.imagePadding = 4;
    UIButton *button = [UIButton buttonWithConfiguration:configuration primaryAction:nil];
    if (action) [button addTarget:self action:action forControlEvents:UIControlEventTouchUpInside];
    return button;
}

- (BOOL)textFieldShouldReturn:(UITextField *)field {
    [self.amountField becomeFirstResponder];
    return NO;
}

- (void)changePeriodUnit {
    NSString *unit = self.periodUnit.selectedSegmentIndex == 1 ? @"year" : @"month";
    self.selectedCustomDate = NO;
    [self script:@"ledgerActions.setPeriodUnit(unit)" arguments:@{@"unit": unit} completion:nil];
}

- (void)previousPeriod { [self movePeriod:@(-1)]; }
- (void)nextPeriod { [self movePeriod:@(1)]; }
- (void)movePeriod:(NSNumber *)step {
    self.selectedCustomDate = NO;
    [self script:@"ledgerActions.movePeriod(step)" arguments:@{@"step": step} completion:nil];
}

- (void)hideSuggestions {
    self.suggestionRequest += 1;
    self.nameSuggestions.hidden = YES;
}

- (void)refreshSuggestions {
    NSString *query = self.nameField.text ?: @"";
    NSInteger request = ++self.suggestionRequest;
    self.nameSuggestions.hidden = YES;
    if (!query.length || !self.nameField.isFirstResponder) return;
    [self script:@"return ledgerActions.suggestNames(kind, query)" arguments:@{@"kind": self.kind, @"query": query}
        completion:^(id result, NSError *error) {
            if (request != self.suggestionRequest || error || !self.nameField.isFirstResponder ||
                ![result isKindOfClass:[NSArray class]]) return;
            NSArray *names = result;
            for (NSUInteger index = 0; index < self.suggestionButtons.count; index++) {
                UIButton *button = self.suggestionButtons[index];
                button.hidden = index >= names.count;
                if (button.hidden) continue;
                UIButtonConfiguration *configuration = button.configuration;
                configuration.title = names[index];
                configuration.titleLineBreakMode = NSLineBreakByTruncatingTail;
                button.configuration = configuration;
            }
            self.nameSuggestions.hidden = names.count == 0;
        }];
}

- (void)useSuggestion:(UIButton *)button {
    self.nameField.text = button.configuration.title;
    [self hideSuggestions];
    [self.amountField becomeFirstResponder];
}

- (void)showEntryCalendar { [self showCalendarForEditor:NO]; }
- (void)showEditCalendar { [self showCalendarForEditor:YES]; }

- (void)showCalendarForEditor:(BOOL)editing {
    UIViewController *host = editing ? self.recordEditor : self;
    if (host.presentedViewController) return;
    [host.view endEditing:YES];
    self.editingCalendar = editing;
    UIViewController *calendar = [UIViewController new];
    calendar.modalPresentationStyle = UIModalPresentationFormSheet;
    calendar.modalInPresentation = YES;
    calendar.view.backgroundColor = [UIColor systemGroupedBackgroundColor];
    calendar.view.tintColor = [UIColor labelColor];

    self.calendarPicker = [UIDatePicker new];
    self.calendarPicker.datePickerMode = UIDatePickerModeDate;
    self.calendarPicker.preferredDatePickerStyle = UIDatePickerStyleInline;
    self.calendarPicker.minimumDate = [self.dateFormat dateFromString:@"1900-01-01"];
    self.calendarPicker.maximumDate = [self.dateFormat dateFromString:@"9999-12-31"];
    NSDate *current = editing ? self.editDate : self.entryDate;
    if (current) self.calendarPicker.date = current;
    self.calendarPicker.accessibilityLabel = @"기록 날짜";

    UIButton *cancel = [self actionButton:@"취소" icon:nil action:@selector(closeCalendar)];
    UIButton *confirm = [self actionButton:@"완료" icon:nil action:@selector(confirmCalendar)];
    UILabel *title = [UILabel new];
    title.text = @"날짜 선택";
    title.font = [UIFont preferredFontForTextStyle:UIFontTextStyleHeadline];
    title.textAlignment = NSTextAlignmentCenter;
    UIStackView *bar = [self row:@[cancel, title, confirm] spacing:8];
    UIStackView *content = [[UIStackView alloc] initWithArrangedSubviews:@[bar, self.calendarPicker]];
    content.axis = UILayoutConstraintAxisVertical;
    content.spacing = 8;
    content.translatesAutoresizingMaskIntoConstraints = NO;
    [calendar.view addSubview:content];
    UILayoutGuide *safe = calendar.view.safeAreaLayoutGuide;
    [NSLayoutConstraint activateConstraints:@[
        [content.topAnchor constraintEqualToAnchor:safe.topAnchor constant:14],
        [content.leadingAnchor constraintEqualToAnchor:safe.leadingAnchor constant:10],
        [content.trailingAnchor constraintEqualToAnchor:safe.trailingAnchor constant:-10],
        [bar.heightAnchor constraintGreaterThanOrEqualToConstant:44],
    ]];
    [host presentViewController:calendar animated:YES completion:nil];
}

- (void)closeCalendar {
    UIViewController *host = self.editingCalendar ? self.recordEditor : self;
    [host.presentedViewController dismissViewControllerAnimated:YES completion:nil];
    self.calendarPicker = nil;
}

- (void)confirmCalendar {
    NSDate *date = self.calendarPicker.date;
    if (self.editingCalendar) {
        self.editDate = date;
        [self showDate:date onButton:self.editDateButton];
    } else {
        self.selectedCustomDate = YES;
        self.entryDate = date;
        [self showDate:date onButton:self.entryDateButton];
    }
    [self closeCalendar];
}

- (void)showDate:(NSDate *)date onButton:(UIButton *)button {
    UIButtonConfiguration *configuration = button.configuration;
    configuration.title = date ? [self.dateFormat stringFromDate:date] : @"날짜";
    configuration.titleLineBreakMode = NSLineBreakByTruncatingTail;
    button.configuration = configuration;
    button.accessibilityValue = date ? configuration.title : nil;
}

- (NSString *)kind { return self.entryKind.selectedSegmentIndex == 1 ? @"income" : @"expense"; }

- (void)changeEntryKind {
    self.nameField.placeholder = self.entryKind.selectedSegmentIndex == 1 ? @"수입 이름" : @"지출 이름";
    self.nameField.accessibilityLabel = self.nameField.placeholder;
    self.categoryID = nil;
    self.categoryLabel = nil;
    [self updateCategoryMenu];
    [self refreshSuggestions];
}

- (void)showIncome { [self showSummary:@"income"]; }
- (void)showExpense { [self showSummary:@"expense"]; }
- (void)showBalance { [self showSummary:@"balance"]; }

- (void)showSummary:(NSString *)key {
    [self.view endEditing:YES];
    [self script:@"ledgerActions.openDetail(key)" arguments:@{@"key": key} completion:nil];
}

- (void)applyPageState:(NSDictionary *)state {
    if (state[@"detail"]) {
        [self showDetailSheet:state[@"detail"]];
        return;
    }
    NSDictionary *totals = state[@"totals"];
    NSArray *keys = @[@"income", @"expense", @"balance"];
    for (NSUInteger index = 0; index < keys.count; index++) {
        UIButton *button = self.summaryButtons[index];
        UIButtonConfiguration *configuration = button.configuration;
        configuration.subtitle = totals[keys[index]];
        configuration.titleAlignment = UIButtonConfigurationTitleAlignmentCenter;
        configuration.baseForegroundColor = [UIColor labelColor];
        button.configuration = configuration;
    }
    BOOL firstState = self.categories == nil;
    self.period.text = state[@"period"];
    self.periodUnit.selectedSegmentIndex = [state[@"mode"] isEqualToString:@"year"] ? 1 : 0;
    self.previousButton.enabled = [state[@"hasPrevious"] boolValue];
    self.nextButton.enabled = [state[@"hasNext"] boolValue];
    self.defaultEntryDate = state[@"date"];
    if (!self.selectedCustomDate) {
        self.entryDate = [self.dateFormat dateFromString:self.defaultEntryDate];
        [self showDate:nil onButton:self.entryDateButton];
    }
    self.categories = state[@"categories"];
    self.ledgerAvailable = ![state[@"loadError"] boolValue];
    self.addButton.enabled = !self.busy && self.ledgerAvailable;
    if (firstState) [self updateCategoryMenu];
    if (!self.ledgerAvailable && firstState) [self showError:@"저장된 기록을 읽지 못했어요. 앱을 다시 열어 주세요."];
}

- (void)showDetailSheet:(NSDictionary *)detail {
    UIViewController *sheet = self.detailSheet;
    if (!sheet) {
        if (![detail[@"present"] boolValue] || self.presentedViewController) return;
        [self.view endEditing:YES];
        sheet = [UIViewController new];
        sheet.modalPresentationStyle = UIModalPresentationFormSheet;
        sheet.sheetPresentationController.prefersGrabberVisible = YES;
        sheet.view.backgroundColor = [UIColor systemGroupedBackgroundColor];
        sheet.view.tintColor = [UIColor labelColor];

        UILabel *title = [self detailLabel:UIFontTextStyleHeadline];
        UILabel *period = [self detailLabel:UIFontTextStyleBody];
        UILabel *total = [self detailLabel:UIFontTextStyleTitle2];
        UILabel *explanation = [self detailLabel:UIFontTextStyleBody];
        UIButton *close = [self actionButton:@"닫기" icon:nil action:@selector(closeDetailSheet)];
        UIStackView *heading = [self row:@[title, close] spacing:8];
        UIStackView *header = [[UIStackView alloc] initWithArrangedSubviews:@[heading, period, total, explanation]];
        header.axis = UILayoutConstraintAxisVertical;
        header.spacing = 6;
        header.translatesAutoresizingMaskIntoConstraints = NO;

        UITableView *table = [[UITableView alloc] initWithFrame:CGRectZero style:UITableViewStyleInsetGrouped];
        table.dataSource = self;
        table.delegate = self;
        table.translatesAutoresizingMaskIntoConstraints = NO;
        [sheet.view addSubview:header];
        [sheet.view addSubview:table];
        UILayoutGuide *safe = sheet.view.safeAreaLayoutGuide;
        [NSLayoutConstraint activateConstraints:@[
            [header.topAnchor constraintEqualToAnchor:safe.topAnchor constant:12],
            [header.leadingAnchor constraintEqualToAnchor:safe.leadingAnchor constant:20],
            [header.trailingAnchor constraintEqualToAnchor:safe.trailingAnchor constant:-20],
            [heading.heightAnchor constraintGreaterThanOrEqualToConstant:44],
            [table.topAnchor constraintEqualToAnchor:header.bottomAnchor constant:8],
            [table.leadingAnchor constraintEqualToAnchor:safe.leadingAnchor],
            [table.trailingAnchor constraintEqualToAnchor:safe.trailingAnchor],
            [table.bottomAnchor constraintEqualToAnchor:safe.bottomAnchor],
        ]];
        self.detailSheet = sheet;
        self.detailTitle = title;
        self.detailPeriod = period;
        self.detailTotal = total;
        self.detailExplanation = explanation;
        self.detailTable = table;
        [self presentViewController:sheet animated:YES completion:nil];
    }
    self.detailTitle.text = detail[@"title"];
    self.detailPeriod.text = detail[@"period"];
    self.detailTotal.text = detail[@"total"];
    self.detailExplanation.text = detail[@"explanation"];
    self.detailExplanation.hidden = !self.detailExplanation.text.length;
    self.detailEntries = detail[@"entries"];
    [self.detailTable reloadData];
}

- (UILabel *)detailLabel:(NSString *)style {
    UILabel *label = [UILabel new];
    label.font = [UIFont preferredFontForTextStyle:style];
    label.adjustsFontForContentSizeCategory = YES;
    label.numberOfLines = 0;
    return label;
}

- (void)closeDetailSheet {
    [self.detailSheet dismissViewControllerAnimated:YES completion:nil];
}

- (NSInteger)tableView:(UITableView *)table numberOfRowsInSection:(NSInteger)section {
    return self.detailEntries.count;
}

- (UITableViewCell *)tableView:(UITableView *)table cellForRowAtIndexPath:(NSIndexPath *)indexPath {
    UITableViewCell *cell = [table dequeueReusableCellWithIdentifier:@"transaction"];
    if (!cell) cell = [[UITableViewCell alloc] initWithStyle:UITableViewCellStyleSubtitle reuseIdentifier:@"transaction"];
    NSDictionary *item = self.detailEntries[indexPath.row];
    cell.textLabel.text = [NSString stringWithFormat:@"%@ · %@", item[@"name"], item[@"displayAmount"]];
    cell.textLabel.numberOfLines = 0;
    cell.detailTextLabel.text = [NSString stringWithFormat:@"%@ · %@", item[@"date"], item[@"categoryLabel"]];
    cell.accessoryType = UITableViewCellAccessoryDisclosureIndicator;
    return cell;
}

- (void)tableView:(UITableView *)table didSelectRowAtIndexPath:(NSIndexPath *)indexPath {
    [table deselectRowAtIndexPath:indexPath animated:YES];
    [self showRecordEditor:self.detailEntries[indexPath.row]];
}

- (void)tableView:(UITableView *)table commitEditingStyle:(UITableViewCellEditingStyle)style
                                      forRowAtIndexPath:(NSIndexPath *)indexPath {
    if (style != UITableViewCellEditingStyleDelete) return;
    NSDictionary *item = self.detailEntries[indexPath.row];
    UIAlertController *alert = [UIAlertController alertControllerWithTitle:@"기록 삭제"
        message:item[@"name"] preferredStyle:UIAlertControllerStyleAlert];
    [alert addAction:[UIAlertAction actionWithTitle:@"취소" style:UIAlertActionStyleCancel handler:nil]];
    [alert addAction:[UIAlertAction actionWithTitle:@"삭제" style:UIAlertActionStyleDestructive handler:^(UIAlertAction *action) {
        [self script:@"return await ledgerActions.deleteRecord(recordID)" arguments:@{@"recordID": item[@"id"]}
            completion:^(id result, NSError *error) {
                if (error || ![result isKindOfClass:[NSString class]] || [result length]) {
                    [self showError:!error && [result isKindOfClass:[NSString class]]
                        ? result : @"삭제하지 못했어요. 다시 시도해 주세요."];
                }
            }];
    }]];
    [self.detailSheet presentViewController:alert animated:YES completion:nil];
}

- (void)showRecordEditor:(NSDictionary *)record {
    UIViewController *host = self.detailSheet ?: self;
    if (self.recordEditor || host.presentedViewController) return;
    self.editID = record[@"id"];
    self.editKind = record[@"kind"];
    self.editCategory = record[@"category"];

    UIViewController *editor = [UIViewController new];
    self.recordEditor = editor;
    editor.modalPresentationStyle = UIModalPresentationFormSheet;
    editor.modalInPresentation = YES;
    editor.view.backgroundColor = [UIColor systemGroupedBackgroundColor];
    editor.view.tintColor = [UIColor labelColor];

    self.cancelEditButton = [self actionButton:@"취소" icon:nil action:@selector(closeRecordEditor)];
    self.saveEditButton = [self actionButton:@"저장" icon:nil action:@selector(saveRecordEdit)];
    UILabel *heading = [UILabel new];
    heading.text = @"기록 수정";
    heading.font = [UIFont preferredFontForTextStyle:UIFontTextStyleHeadline];
    heading.textAlignment = NSTextAlignmentCenter;
    UIStackView *top = [self row:@[self.cancelEditButton, heading, self.saveEditButton] spacing:8];
    self.editNameField = [self textFieldWithPlaceholder:@"이름"];
    self.editNameField.text = record[@"name"];
    self.editAmountField = [self textFieldWithPlaceholder:@"금액 (원)"];
    self.editAmountField.keyboardType = UIKeyboardTypeNumberPad;
    self.editAmountField.text = record[@"amount"];
    self.editCategoryButton = [self actionButton:@"카테고리" icon:@"tag" action:NULL];
    self.editCategoryButton.showsMenuAsPrimaryAction = YES;
    [self refreshEditCategories];

    self.editDate = [self.dateFormat dateFromString:record[@"date"]];
    self.editDateButton = [self actionButton:@"날짜" icon:@"calendar" action:@selector(showEditCalendar)];
    self.editDateButton.accessibilityLabel = @"기록 날짜";
    [self showDate:self.editDate onButton:self.editDateButton];

    self.editError = [UILabel new];
    self.editError.font = [UIFont preferredFontForTextStyle:UIFontTextStyleBody];
    self.editError.numberOfLines = 0;
    self.editError.hidden = YES;
    UIStackView *form = [[UIStackView alloc] initWithArrangedSubviews:
        @[top, self.editNameField, self.editAmountField, self.editCategoryButton, self.editDateButton, self.editError]];
    form.axis = UILayoutConstraintAxisVertical;
    form.spacing = 8;
    form.translatesAutoresizingMaskIntoConstraints = NO;
    [editor.view addSubview:form];
    UILayoutGuide *safe = editor.view.safeAreaLayoutGuide;
    [NSLayoutConstraint activateConstraints:@[
        [form.topAnchor constraintEqualToAnchor:safe.topAnchor constant:12],
        [form.leadingAnchor constraintEqualToAnchor:safe.leadingAnchor constant:18],
        [form.trailingAnchor constraintEqualToAnchor:safe.trailingAnchor constant:-18],
        [top.heightAnchor constraintGreaterThanOrEqualToConstant:44],
        [self.editNameField.heightAnchor constraintGreaterThanOrEqualToConstant:44],
        [self.editAmountField.heightAnchor constraintGreaterThanOrEqualToConstant:44],
        [self.editCategoryButton.heightAnchor constraintGreaterThanOrEqualToConstant:44],
        [self.editDateButton.heightAnchor constraintGreaterThanOrEqualToConstant:44],
    ]];
    [host presentViewController:editor animated:YES completion:nil];
}

- (void)refreshEditCategories {
    NSArray *items = self.categories[self.editKind];
    NSMutableArray *options = [NSMutableArray new];
    UIButtonConfiguration *configuration = self.editCategoryButton.configuration;
    __weak LedgerController *owner = self;
    for (NSUInteger index = 0; index < items.count; index++) {
        NSDictionary *item = items[index];
        NSString *identifier = item[@"id"];
        UIAction *action = [UIAction actionWithTitle:item[@"label"] image:nil identifier:nil
            handler:^(UIAction *selection) {
                owner.editCategory = identifier;
                [owner refreshEditCategories];
            }];
        if ([identifier isEqualToString:self.editCategory]) {
            action.state = UIMenuElementStateOn;
            configuration.title = item[@"label"];
        }
        [options addObject:action];
    }
    self.editCategoryButton.configuration = configuration;
    self.editCategoryButton.menu = [UIMenu menuWithTitle:@"카테고리" children:options];
}

- (void)closeRecordEditor {
    [self.recordEditor dismissViewControllerAnimated:YES completion:nil];
    self.recordEditor = nil;
}

- (void)saveRecordEdit {
    NSDictionary *input = @{
        @"id": self.editID, @"name": self.editNameField.text ?: @"",
        @"amount": self.editAmountField.text ?: @"", @"category": self.editCategory ?: @"",
        @"date": [self.dateFormat stringFromDate:self.editDate] ?: @"",
    };
    self.saveEditButton.enabled = NO;
    self.cancelEditButton.enabled = NO;
    [self.recordEditor.view endEditing:YES];
    [self script:@"return await ledgerActions.updateRecord(input)" arguments:@{@"input": input}
        completion:^(id result, NSError *error) {
            if (!error && [result isKindOfClass:[NSString class]] && [result isEqualToString:@""]) {
                [self closeRecordEditor];
                return;
            }
            self.saveEditButton.enabled = YES;
            self.cancelEditButton.enabled = YES;
            self.editError.text = !error && [result isKindOfClass:[NSString class]]
                ? result : @"수정 내용을 저장하지 못했어요. 다시 시도해 주세요.";
            self.editError.hidden = NO;
        }];
}

- (void)updateCategoryMenu {
    NSArray *categoryList = self.categories[self.kind];
    NSMutableArray *options = [NSMutableArray new];
    __weak LedgerController *controller = self;
    for (NSUInteger index = 0; index < categoryList.count; index++) {
        NSDictionary *item = categoryList[index];
        NSString *label = item[@"label"];
        NSString *identifier = item[@"id"];
        UIAction *action = [UIAction actionWithTitle:label image:nil identifier:nil handler:^(UIAction *selected) {
            LedgerController *owner = controller;
            owner.categoryID = identifier;
            owner.categoryLabel = label;
            [owner updateCategoryMenu];
            [owner.view endEditing:YES];
        }];
        if ([self.categoryID isEqualToString:identifier]) action.state = UIMenuElementStateOn;
        [options addObject:action];
    }
    UIButtonConfiguration *configuration = self.categoryButton.configuration;
    configuration.title = self.categoryLabel ?: @"카테고리";
    self.categoryButton.configuration = configuration;
    self.categoryButton.menu = [UIMenu menuWithTitle:@"카테고리" children:options];
    self.categoryButton.enabled = options.count > 0;
}

- (void)addEntry {
    if (self.busy) return;
    if (!self.categoryID) {
        [self showError:@"카테고리를 선택해 주세요."];
        return;
    }
    NSString *date = self.selectedCustomDate ? [self.dateFormat stringFromDate:self.entryDate] : self.defaultEntryDate;
    NSDictionary *input = @{@"name": self.nameField.text ?: @"", @"amount": self.amountField.text ?: @"",
                            @"category": self.categoryID, @"date": date ?: @"", @"kind": self.kind};
    self.busy = YES;
    self.addButton.enabled = NO;
    self.nameField.enabled = NO;
    self.amountField.enabled = NO;
    self.categoryButton.enabled = NO;
    self.entryKind.enabled = NO;
    [self.view endEditing:YES];
    [self script:@"return await ledgerActions.addRecord(input)" arguments:@{@"input": input}
        completion:^(id result, NSError *error) {
            self.busy = NO;
            self.addButton.enabled = self.ledgerAvailable;
            self.nameField.enabled = YES;
            self.amountField.enabled = YES;
            self.categoryButton.enabled = YES;
            self.entryKind.enabled = YES;
            NSString *message = [result isKindOfClass:[NSString class]] ? result : @"저장하지 못했어요. 다시 시도해 주세요.";
            if (error || ![message isEqualToString:@""]) {
                [self showError:message];
                return;
            }
            self.nameField.text = @"";
            self.amountField.text = @"";
            self.selectedCustomDate = NO;
            self.entryDate = [self.dateFormat dateFromString:self.defaultEntryDate];
            [self showDate:nil onButton:self.entryDateButton];
            [self changeEntryKind];
        }];
}

- (void)script:(NSString *)body arguments:(NSDictionary *)arguments completion:(void (^)(id, NSError *))completion {
    [self.web callAsyncJavaScript:body arguments:arguments inFrame:nil inContentWorld:[WKContentWorld pageWorld] completionHandler:completion];
}

- (void)showError:(NSString *)message {
    UIAlertController *alert = [UIAlertController alertControllerWithTitle:nil message:message preferredStyle:UIAlertControllerStyleAlert];
    [alert addAction:[UIAlertAction actionWithTitle:@"확인" style:UIAlertActionStyleCancel handler:nil]];
    [(self.detailSheet ?: self) presentViewController:alert animated:YES completion:nil];
}
