var mediaWikiLoadStart=(new Date()).getTime(),mwPerformance=(window.performance&&performance.mark)?performance:{mark:function(){}};mwPerformance.mark('mwLoadStart');function isCompatible(ua){if(ua===undefined){ua=navigator.userAgent;}return!((ua.indexOf('MSIE')!==-1&&parseFloat(ua.split('MSIE')[1])<8)||(ua.indexOf('Firefox/')!==-1&&parseFloat(ua.split('Firefox/')[1])<3)||(ua.indexOf('Opera/')!==-1&&(ua.indexOf('Version/')===-1?parseFloat(ua.split('Opera/')[1])<10:parseFloat(ua.split('Version/')[1])<12))||(ua.indexOf('Opera ')!==-1&&parseFloat(ua.split(' Opera ')[1])<10)||ua.match(/BlackBerry[^\/]*\/[1-5]\./)||ua.match(/webOS\/1\.[0-4]/)||ua.match(/PlayStation/i)||ua.match(/SymbianOS|Series60/)||ua.match(/NetFront/)||ua.match(/Opera Mini/)||ua.match(/S40OviBrowser/)||ua.match(/MeeGo/)||(ua.match(/Glass/)&&ua.match(/Android/)));}(function(){if(!isCompatible()){document.documentElement.className=document.documentElement.className.replace(/(^|\s)client-js(\s|$)/,'$1client-nojs$2');return;}
function startUp(){mw.config=new mw.Map(true);mw.loader.addSource({"local":"/load.php"});mw.loader.register([["site","X9KNoDKo"],["noscript","i7Im0Igw",[],"noscript"],["filepage","Fow0SXQl"],["user.groups","5ZG9EoYr",[],"user"],["user","O71taaj5",[],"user"],["user.cssprefs","64Nx0RWw",[],"private"],["user.defaults","hY5NkNwh"],["user.options","+JoudQIu",[6],"private"],["user.tokens","BWQTEyEx",[],"private"],["mediawiki.language.data","pJtq8AIz",[168]],["mediawiki.skinning.elements","t7ZN/mtd"],["mediawiki.skinning.content","JA7UN86Q"],["mediawiki.skinning.interface","I6sZnvTU"],["mediawiki.skinning.content.parsoid","LyUw7b/e"],["mediawiki.skinning.content.externallinks","ra2xWBGq"],["jquery.accessKeyLabel","CuGkv0H6",[25,129]],["jquery.appear","3t/fGO8a"],["jquery.arrowSteps","6pQwBiWd"],["jquery.async","L9yq7XZx"],["jquery.autoEllipsis","Syp3tPvY",[37]],["jquery.badge","9ReveUkh",[165]],["jquery.byteLength","gAVkMeHR"],["jquery.byteLimit","reyQB5DL",[21]],["jquery.checkboxShiftClick",
"26Pac5Cx"],["jquery.chosen","lm+89Gjz"],["jquery.client","bBgKWCMy"],["jquery.color","xr/XkYO8",[27]],["jquery.colorUtil","UPXNtRqk"],["jquery.confirmable","6ZblJZDo",[169]],["jquery.cookie","biihrK+M"],["jquery.expandableField","VMvYn8rM"],["jquery.farbtastic","axZr20o7",[27]],["jquery.footHovzer","Lh9fbnT6"],["jquery.form","YI17jcJC"],["jquery.fullscreen","5fGtGZ7R"],["jquery.getAttrs","FTbJeynk"],["jquery.hidpi","K2JeFz0k"],["jquery.highlightText","+1gVQWdp",[227,129]],["jquery.hoverIntent","WUx6usGp"],["jquery.i18n","hrYV29as",[167]],["jquery.localize","GzDg4DD6"],["jquery.makeCollapsible","Hp3C1OId"],["jquery.mockjax","89D9uWMl"],["jquery.mw-jump","kH9I92Tk"],["jquery.mwExtension","gv26pB4M"],["jquery.placeholder","LMFpyJzQ"],["jquery.qunit","Jl2q1p5m"],["jquery.qunit.completenessTest","TfI9qEwa",[46]],["jquery.spinner","OWmSoxrP"],["jquery.jStorage","dIqmPslm",[93]],["jquery.suggestions","IPKhtn+j",[37]],["jquery.tabIndex","4KdcmUd8"],["jquery.tablesorter","LPzOuN1C",[227,129,
170]],["jquery.textSelection","BNFxGU7h",[25]],["jquery.throttle-debounce","zWyUDKce"],["jquery.validate","WIOmcHBz"],["jquery.xmldom","0yP8PyAa"],["jquery.tipsy","oUojSL2o"],["jquery.ui.core","obNDhd5j",[59],"jquery.ui"],["jquery.ui.core.styles","b4RUNZp+",[],"jquery.ui"],["jquery.ui.accordion","nV9ScCK/",[58,78],"jquery.ui"],["jquery.ui.autocomplete","d/URrZN2",[67],"jquery.ui"],["jquery.ui.button","gyJCAYk1",[58,78],"jquery.ui"],["jquery.ui.datepicker","Odk4Cnm4",[58],"jquery.ui"],["jquery.ui.dialog","E63PR2aH",[62,65,69,71],"jquery.ui"],["jquery.ui.draggable","Vh8BPkJ+",[58,68],"jquery.ui"],["jquery.ui.droppable","qR5uyDrq",[65],"jquery.ui"],["jquery.ui.menu","G28DesK/",[58,69,78],"jquery.ui"],["jquery.ui.mouse","G1qZ970n",[78],"jquery.ui"],["jquery.ui.position","Idd4xV4l",[],"jquery.ui"],["jquery.ui.progressbar","motvVfRg",[58,78],"jquery.ui"],["jquery.ui.resizable","YjFbJZx6",[58,68],"jquery.ui"],["jquery.ui.selectable","3LB4Pix+",[58,68],"jquery.ui"],["jquery.ui.slider",
"cZxgjOyc",[58,68],"jquery.ui"],["jquery.ui.sortable","uC7+WdGt",[58,68],"jquery.ui"],["jquery.ui.spinner","bTpfvhpu",[62],"jquery.ui"],["jquery.ui.tabs","HIR97i3f",[58,78],"jquery.ui"],["jquery.ui.tooltip","BRGLvMY5",[58,69,78],"jquery.ui"],["jquery.ui.widget","YljecAea",[],"jquery.ui"],["jquery.effects.core","y+Od14dC",[],"jquery.ui"],["jquery.effects.blind","VL2VsrWQ",[79],"jquery.ui"],["jquery.effects.bounce","kIWiDVAi",[79],"jquery.ui"],["jquery.effects.clip","k3or6dRr",[79],"jquery.ui"],["jquery.effects.drop","uaflExZ5",[79],"jquery.ui"],["jquery.effects.explode","I2m5DZYG",[79],"jquery.ui"],["jquery.effects.fade","3fyFUuim",[79],"jquery.ui"],["jquery.effects.fold","ycqrfPOe",[79],"jquery.ui"],["jquery.effects.highlight","9Fq0FRvc",[79],"jquery.ui"],["jquery.effects.pulsate","OylHcYuj",[79],"jquery.ui"],["jquery.effects.scale","oBIx2Bjb",[79],"jquery.ui"],["jquery.effects.shake","REM8CAaW",[79],"jquery.ui"],["jquery.effects.slide","nqTYGSba",[79],"jquery.ui"],[
"jquery.effects.transfer","8MP/BOcA",[79],"jquery.ui"],["json","nMFbZKC4",[],null,null,"return!!(window.JSON\u0026\u0026JSON.stringify\u0026\u0026JSON.parse);"],["moment","HesMK3Ot"],["mediawiki.apihelp","UcZiCY6y",[119]],["mediawiki.template","liPO+luj"],["mediawiki.template.mustache","R7rPxM/l",[96]],["mediawiki.template.regexp","TERvQXLR",[96]],["mediawiki.apipretty","7IOcETLG"],["mediawiki.api","Pvx9u7cp",[145,8]],["mediawiki.api.category","U0D2o5gv",[134,100]],["mediawiki.api.edit","vRLnrpI+",[134,100]],["mediawiki.api.login","LNLI8fv1",[100]],["mediawiki.api.options","zxcPkiXL",[100]],["mediawiki.api.parse","uVtHFNFR",[100]],["mediawiki.api.upload","0ZHzZFPU",[227,93,102]],["mediawiki.api.watch","m33pPvD7",[100]],["mediawiki.content.json","bNcwLjHB"],["mediawiki.confirmCloseWindow","Jummyj7M"],["mediawiki.debug","0ku62KkJ",[32,57]],["mediawiki.debug.init","REq6VO5U",[110]],["mediawiki.feedback","TjPSXspC",[134,125,229]],["mediawiki.feedlink","cGGEALcU"],["mediawiki.filewarning",
"hJjKoPPx",[229]],["mediawiki.ForeignApi","89Qu5ftS",[116]],["mediawiki.ForeignApi.core","AkcN8j/J",[100,228]],["mediawiki.helplink","gwIHL3jJ"],["mediawiki.hidpi","qTj2fa7/",[36],null,null,"return'srcset'in new Image();"],["mediawiki.hlist","WKgGRTr4",[25]],["mediawiki.htmlform","gHbJwYlV",[22,129]],["mediawiki.htmlform.styles","lL7SqbCp"],["mediawiki.htmlform.ooui.styles","QicRcRQm"],["mediawiki.icon","+1e17qw4"],["mediawiki.inspect","n4NpFGcC",[21,93,129]],["mediawiki.messagePoster","JSzwU9TU",[100,228]],["mediawiki.messagePoster.wikitext","BQypAM8h",[102,125]],["mediawiki.notification","kl+0LD3R",[177]],["mediawiki.notify","3cuMunPz"],["mediawiki.RegExp","UJARFDks"],["mediawiki.pager.tablePager","bWV8LIH9"],["mediawiki.searchSuggest","z3V3QXrH",[35,45,50,100]],["mediawiki.sectionAnchor","z/yOPMWe"],["mediawiki.storage","10/z3uoD"],["mediawiki.Title","af7TbEAJ",[21,145]],["mediawiki.Upload","4qxkNrSr",[106]],["mediawiki.ForeignUpload","E3Rx5Sq0",[115,135]],[
"mediawiki.ForeignStructuredUpload","Okl5qqtP",[136]],["mediawiki.Upload.Dialog","0tYMAynQ",[139]],["mediawiki.Upload.BookletLayout","6ajm3HUR",[135,169,229]],["mediawiki.ForeignStructuredUpload.BookletLayout","DTjdM1oR",[137,139,224,223]],["mediawiki.toc","rgptO15q",[146]],["mediawiki.Uri","u3gk9lmz",[145,98]],["mediawiki.user","FD800QgH",[100,146,7]],["mediawiki.userSuggest","07waCb9b",[50,100]],["mediawiki.util","7BQ/3TMX",[15,128]],["mediawiki.cookie","D2Lib1sX",[29]],["mediawiki.toolbar","5RsVMdEi"],["mediawiki.experiments","NeR2ZcWU"],["mediawiki.action.edit","gv3Ixcjy",[22,53,150]],["mediawiki.action.edit.styles","9YlzupMt"],["mediawiki.action.edit.collapsibleFooter","/37CyUpV",[41,146,123]],["mediawiki.action.edit.preview","IUl+UKCC",[33,48,53,155,100,169]],["mediawiki.action.edit.stash","xQn+wf9w",[35,100]],["mediawiki.action.history","TbboHgDU"],["mediawiki.action.history.diff","fZif8K1r"],["mediawiki.action.view.dblClickEdit","ZZSPCr2Y",[177,7]],[
"mediawiki.action.view.metadata","Bz/qAx9y"],["mediawiki.action.view.categoryPage.styles","40hmHUA7"],["mediawiki.action.view.postEdit","KeP2ZlF0",[146,169,96]],["mediawiki.action.view.redirect","FlvQbpEi",[25]],["mediawiki.action.view.redirectPage","eNpUv3eh"],["mediawiki.action.view.rightClickEdit","QDiF9O/l"],["mediawiki.action.edit.editWarning","QeoBclbY",[53,109,169]],["mediawiki.action.view.filepage","VyN+5X8n"],["mediawiki.language","ubeH/NSz",[166,9]],["mediawiki.cldr","m6byevRj",[167]],["mediawiki.libs.pluralruleparser","re7aJ0ro"],["mediawiki.language.init","f1rsH8Ws"],["mediawiki.jqueryMsg","EjtXgyaf",[227,165,145,7]],["mediawiki.language.months","EP3hX79I",[165]],["mediawiki.language.names","PEZBZtR9",[168]],["mediawiki.language.specialCharacters","lfy9K43n",[165]],["mediawiki.libs.jpegmeta","8tf+xIkf"],["mediawiki.page.gallery","RPjEhR5Z",[54,175]],["mediawiki.page.gallery.styles","rzO2PvrX"],["mediawiki.page.ready","QElF4yeu",[15,23,41,43,45]],["mediawiki.page.startup",
"CdpByrjh",[145]],["mediawiki.page.patrol.ajax","MNCt7AVx",[48,134,100,177]],["mediawiki.page.watch.ajax","4BID9QkT",[107,177]],["mediawiki.page.image.pagination","pClSFUxm",[48,142]],["mediawiki.special","tHCOU0aY"],["mediawiki.special.block","k0NfjLJ6",[145]],["mediawiki.special.changeemail","yeRlxYnk",[145]],["mediawiki.special.changeslist","SYssqOlC"],["mediawiki.special.changeslist.legend","Jyp5ju2b"],["mediawiki.special.changeslist.legend.js","E8G6CLVz",[41,146]],["mediawiki.special.changeslist.enhanced","WG5WzkQz"],["mediawiki.special.edittags","3SerGpUy",[24]],["mediawiki.special.edittags.styles","CX0zrPyt"],["mediawiki.special.import","mHz6ghYK"],["mediawiki.special.movePage","NDN8RYK5",[221]],["mediawiki.special.movePage.styles","umfVhJhx"],["mediawiki.special.pageLanguage","z922aUx2"],["mediawiki.special.pagesWithProp","CB4GoV+X"],["mediawiki.special.preferences","9eJ0nuke",[109,165,127]],["mediawiki.special.recentchanges","jQ0omsZU",[181]],["mediawiki.special.search",
"COXFEvp9"],["mediawiki.special.undelete","NRL3ykKw"],["mediawiki.special.upload","q1gIgpAk",[48,134,100,109,169,173,96]],["mediawiki.special.userlogin.common.styles","XUwa6Jvp"],["mediawiki.special.userlogin.signup.styles","C9ctYmJC"],["mediawiki.special.userlogin.login.styles","SbLisAB7"],["mediawiki.special.userlogin.signup.js","XXU1dLr3",[54,100,169]],["mediawiki.special.unwatchedPages","yDLIUui6",[134,107]],["mediawiki.special.javaScriptTest","F3rD1fJW",[142]],["mediawiki.special.version","Uuj++IU+"],["mediawiki.legacy.config","v8D2mIsU"],["mediawiki.legacy.commonPrint","JsdGhCVm"],["mediawiki.legacy.protect","31ymFtjt",[22]],["mediawiki.legacy.shared","HT7hC6ut"],["mediawiki.legacy.oldshared","9hMnF14G"],["mediawiki.legacy.wikibits","MN9VgIdo",[145]],["mediawiki.ui","JDymfVPU"],["mediawiki.ui.checkbox","kgGE+f/J"],["mediawiki.ui.radio","o/BxO6cm"],["mediawiki.ui.anchor","Rw7o/0Xr"],["mediawiki.ui.button","lEqtP+9l"],["mediawiki.ui.input","RK9h3S6L"],["mediawiki.ui.icon",
"YEHBmDsN"],["mediawiki.ui.text","KqkvnxDz"],["mediawiki.widgets","rZm59eeB",[19,22,115,134,224,222]],["mediawiki.widgets.styles","MsXG36F/"],["mediawiki.widgets.DateInputWidget","sknx17JY",[94,229]],["mediawiki.widgets.CategorySelector","RoRoSdO+",[100,229]],["mediawiki.widgets.UserInputWidget","1mwjWYtY",[229]],["es5-shim","qvtPW3iJ",[],null,null,"return(function(){'use strict';return!this\u0026\u0026!!Function.prototype.bind;}());"],["dom-level2-shim","xFOyO9j6",[],null,null,"return!!window.Node;"],["oojs","W4ixwOkI",[226,93]],["oojs-ui","8iaW/BBo",[228,230,231,232,233]],["oojs-ui.styles","6QW6m1wE"],["oojs-ui.styles.icons","bmXEWgFV"],["oojs-ui.styles.indicators","nH2fWZpa"],["oojs-ui.styles.textures","seoBrvdn"],["oojs-ui.styles.icons-accessibility","2h7OpCVF"],["oojs-ui.styles.icons-alerts","WDsqLepI"],["oojs-ui.styles.icons-content","qXcGKAJE"],["oojs-ui.styles.icons-editing-advanced","ZNbXSxTa"],["oojs-ui.styles.icons-editing-core","izU5ri4G"],[
"oojs-ui.styles.icons-editing-list","EXQP6lXy"],["oojs-ui.styles.icons-editing-styling","4ZXz5nMd"],["oojs-ui.styles.icons-interactions","IsVZYt1P"],["oojs-ui.styles.icons-layout","vYZseoDy"],["oojs-ui.styles.icons-location","x8c0E8Rr"],["oojs-ui.styles.icons-media","4vdk3Nr4"],["oojs-ui.styles.icons-moderation","m84Nh5og"],["oojs-ui.styles.icons-movement","kKqN7PcC"],["oojs-ui.styles.icons-user","CFELo6ev"],["oojs-ui.styles.icons-wikimedia","+8zPDP9J"],["skins.cologneblue","iGT0vgwm"],["skins.modern","r+sY3DdT"],["skins.monobook.styles","WQAO/OPV"],["skins.vector.styles","7Mz5ouQ6"],["skins.vector.styles.responsive","taPkxxXg"],["skins.vector.js","mFmbDUt/",[51,54]]]);;mw.config.set({"wgLoadScript":"/load.php","debug":!1,"skin":"vector","stylepath":"/skins","wgUrlProtocols":
"bitcoin\\:|ftp\\:\\/\\/|ftps\\:\\/\\/|geo\\:|git\\:\\/\\/|gopher\\:\\/\\/|http\\:\\/\\/|https\\:\\/\\/|irc\\:\\/\\/|ircs\\:\\/\\/|magnet\\:|mailto\\:|mms\\:\\/\\/|news\\:|nntp\\:\\/\\/|redis\\:\\/\\/|sftp\\:\\/\\/|sip\\:|sips\\:|sms\\:|ssh\\:\\/\\/|svn\\:\\/\\/|tel\\:|telnet\\:\\/\\/|urn\\:|worldwind\\:\\/\\/|xmpp\\:|\\/\\/","wgArticlePath":"/index.php?title=$1","wgScriptPath":"","wgScriptExtension":".php","wgScript":"/index.php","wgSearchType":null,"wgVariantArticlePath":!1,"wgActionPaths":{},"wgServer":"http://wiki.stm32duino.com","wgServerName":"wiki.stm32duino.com","wgUserLanguage":"en","wgContentLanguage":"en","wgTranslateNumerals":!0,"wgVersion":"1.26.2","wgEnableAPI":!0,"wgEnableWriteAPI":!0,"wgMainPageTitle":"Main Page","wgFormattedNamespaces":{"-2":"Media","-1":"Special","0":"","1":"Talk","2":"User","3":"User talk","4":"STM32duino wiki","5":"STM32duino wiki talk","6":"File","7":"File talk","8":"MediaWiki","9":"MediaWiki talk","10":"Template","11":"Template talk","12"
:"Help","13":"Help talk","14":"Category","15":"Category talk"},"wgNamespaceIds":{"media":-2,"special":-1,"":0,"talk":1,"user":2,"user_talk":3,"stm32duino_wiki":4,"stm32duino_wiki_talk":5,"file":6,"file_talk":7,"mediawiki":8,"mediawiki_talk":9,"template":10,"template_talk":11,"help":12,"help_talk":13,"category":14,"category_talk":15,"image":6,"image_talk":7,"project":4,"project_talk":5},"wgContentNamespaces":[0],"wgSiteName":"STM32duino wiki","wgDBname":"synergie_stm32duinowiki","wgExtraSignatureNamespaces":[],"wgAvailableSkins":{"cologneblue":"CologneBlue","modern":"Modern","monobook":"MonoBook","vector":"Vector","fallback":"Fallback","apioutput":"ApiOutput"},"wgExtensionAssetsPath":"/extensions","wgCookiePrefix":"synergie_stm32duinowiki","wgCookieDomain":"","wgCookiePath":"/","wgCookieExpiration":15552000,"wgResourceLoaderMaxQueryLength":2000,"wgCaseSensitiveNamespaces":[],"wgLegalTitleChars":" %!\"$&'()*,\\-./0-9:;=?@A-Z\\\\\\^_`a-z~+\\u0080-\\uFFFF","wgResourceLoaderStorageVersion":
1,"wgResourceLoaderStorageEnabled":!1,"wgResourceLoaderLegacyModules":["mediawiki.legacy.wikibits"],"wgForeignUploadTargets":[],"wgEnableUploads":!0});window.RLQ=window.RLQ||[];while(RLQ.length){RLQ.shift()();}window.RLQ={push:function(fn){fn();}};}var script=document.createElement('script');script.src="/load.php?debug=false&lang=en&modules=jquery%2Cmediawiki&only=scripts&skin=vector&version=qcauIx9Z";script.onload=script.onreadystatechange=function(){if(!script.readyState||/loaded|complete/.test(script.readyState)){script.onload=script.onreadystatechange=null;script=null;startUp();}};document.getElementsByTagName('head')[0].appendChild(script);}());