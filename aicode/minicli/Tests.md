# Task Test Methods

## Purpose
- Record test methods and evidence after each task is implemented.
- Make testing repeatable across Windows and Linux.

## Update Rules
- Update this file immediately after completing each `Txx`.
- Include command, environment, expected result, and actual result.
- Attach failure notes and fix reference when a test fails.

## Global Test Baseline
- Unit tests: include normal, error, and boundary cases.
- Integration tests: verify module interactions for the task scope.
- E2E smoke: run at key flow points (`startup`, `chat`, `slash`, `run`).
- Coverage target:
  - Core code introduced by the task: `>= 90%`
  - Overall project: `>= 80%`

## Command Template
```bash
# type check
npm run typecheck

# unit
npm run test:unit

# integration
npm run test:integration

# e2e smoke
npm run test:e2e:smoke

# coverage
npm run test:coverage
```

## Task Test Record Template
Use this section structure for each task:

```md
### Txx - Task Name
- Date:
- Env: (Windows/Linux, Node version)
- Scope:
- Unit Cases:
  - [ ] case-1
  - [ ] case-2
- Integration Cases:
  - [ ] case-1
- E2E Smoke:
  - [ ] case-1
- Commands:
  - `...`
- Expected:
- Actual:
- Coverage:
  - Core:
  - Overall:
- Result: PASS/FAIL
- Notes:
```

## Task Test Entries

### T01 - 初始化 CLI 工程骨架
- Date: 2026-02-10
- Env: Windows 11, Node v22.17.1, npm 11.9.0
- Scope: CLI init, entrypoint, `--version`
- Unit Cases:
  - [x] 版本字符串格式正确
  - [x] 入口参数解析正确
  - [x] 非法参数返回错误码
  - [x] 空参数行为正确
  - [x] 平台分支判断正确
  - [x] `--version` 输出非空
- Integration Cases:
  - [x] `minicli --version` 退出码为 0
- E2E Smoke:
  - [x] 编译后入口可执行并输出版本号
- Commands:
  - `npm run typecheck`
  - `npm test`
  - `npm run build; node build/src/index.js --version`
- Expected: T01 scope features compile and run; unit + integration pass; version command exits 0 with non-empty output.
- Actual: All unit tests (9) and integration test (1) passed. `node build/src/index.js --version` prints `minicli 0.1.0`.
- Coverage:
  - Core: N/A (coverage tooling not added in T01)
  - Overall: N/A (coverage tooling not added in T01)
- Result: PASS
- Notes: Node test runner uses `--experimental-test-isolation=none` to avoid sandbox spawn restrictions.

### T02 - REPL 空循环
- Date:
- Env:
- Scope: REPL loop, echo, Ctrl+C exit
- Unit Cases:
  - [ ] 杈撳叆瑙ｆ瀽
  - [ ] 绌鸿蹇界暐
  - [ ] 瓒呴暱杈撳叆澶勭悊
  - [ ] EOF 澶勭悊
  - [ ] 涓柇淇″彿澶勭悊
  - [ ] 杈撳嚭缂撳啿澶勭悊
- Integration Cases:
  - [ ] 瀛愯繘绋嬭緭鍏ヨ緭鍑哄洖鏄句竴鑷?- E2E Smoke:
  - [ ] 鍚姩 -> 杈撳叆 -> 閫€鍑?- Commands:
- Expected:
- Actual:
- Coverage:
  - Core:
  - Overall:
- Result:
- Notes:

### T03 - /help 涓?/exit
- Date:
- Env:
- Scope: command routing, help, exit
- Unit Cases:
  - [ ] 鍛戒护鍖归厤
  - [ ] 鏈煡鍛戒护鎻愮ず
  - [ ] help 鍐呭瀹屾暣
  - [ ] exit 閫€鍑烘爣蹇?  - [ ] 绌虹櫧瀛楃澶勭悊
  - [ ] 澶у皬鍐欑瓥鐣?- Integration Cases:
  - [ ] `/help -> /exit` 閾捐矾
- E2E Smoke:
  - [ ] 浜や簰閫€鍑烘垚鍔?- Commands:
- Expected:
- Actual:
- Coverage:
  - Core:
  - Overall:
- Result:
- Notes:

### T04 - 鍙屽眰閰嶇疆鍔犺浇
- Date:
- Env:
- Scope: global/project config merge
- Unit Cases:
  - [ ] 鍏ㄥ眬璺緞瑙ｆ瀽
  - [ ] 椤圭洰璺緞瑙ｆ瀽
  - [ ] 瑕嗙洊浼樺厛绾?  - [ ] 缂哄け鏂囦欢闄嶇骇
  - [ ] 绌洪厤缃鐞?  - [ ] 娉ㄩ噴琛屽鐞?- Integration Cases:
  - [ ] 鍙岄厤缃鐩栫粨鏋滄纭?- E2E Smoke:
  - [ ] 鍚姩鏃跺姞杞介厤缃垚鍔?- Commands:
- Expected:
- Actual:
- Coverage:
  - Core:
  - Overall:
- Result:
- Notes:

### T05 - 閰嶇疆鏍￠獙涓庢姤閿?- Date:
- Env:
- Scope: schema validation and errors
- Unit Cases:
  - [ ] 蹇呭～缂哄け
  - [ ] 瀛楁绫诲瀷閿欒
  - [ ] 闈炴硶鏋氫妇鍊?  - [ ] 鏈煡瀛楁澶勭悊
  - [ ] 榛樿鍊兼敞鍏?  - [ ] 閿欒鏂囨鍙
- Integration Cases:
  - [ ] 鎹熷潖閰嶇疆鏃跺彲鎻愮ず骞剁户缁?- E2E Smoke:
  - [ ] 閿欓厤閰嶇疆涓嬪惎鍔ㄨ涓哄彲棰勬祴
- Commands:
- Expected:
- Actual:
- Coverage:
  - Core:
  - Overall:
- Result:
- Notes:

### T06 - LLMProvider 鎺ュ彛涓?Mock
- Date:
- Env:
- Scope: provider contract and mock
- Unit Cases:
  - [ ] 鎺ュ彛濂戠害鏍￠獙
  - [ ] 璇锋眰缁撴瀯鏍￠獙
  - [ ] 杩斿洖缁撴瀯鏍￠獙
  - [ ] 寮傚父浼犻€?  - [ ] 绌烘秷鎭鐞?  - [ ] 澶氳疆娑堟伅鎷艰
- Integration Cases:
  - [ ] REPL 璋冪敤 mock provider 鎴愬姛
- E2E Smoke:
  - [ ] mock 鑱婂ぉ娴佺▼鍙繍琛?- Commands:
- Expected:
- Actual:
- Coverage:
  - Core:
  - Overall:
- Result:
- Notes:

### T07 - GLM OpenAI-compatible 閫氳矾
- Date:
- Env:
- Scope: glm provider http path
- Unit Cases:
  - [ ] 璇锋眰澶存瀯閫?  - [ ] URL 鎷兼帴
  - [ ] 榛樿妯″瀷鍊?  - [ ] 鍝嶅簲鏄犲皠
  - [ ] 绌?choices 澶勭悊
  - [ ] 缃戠粶閿欒鍖呰
- Integration Cases:
  - [ ] 璇锋眰 mock API 鎴愬姛
- E2E Smoke:
  - [ ] provider 鍏ㄩ摼璺彲杩愯
- Commands:
- Expected:
- Actual:
- Coverage:
  - Core:
  - Overall:
- Result:
- Notes:

### T08 - /login 涓?/model
- Date:
- Env:
- Scope: key storage and model inspect
- Unit Cases:
  - [ ] key 淇濆瓨璇诲彇
  - [ ] key 鎺╃爜鏄剧ず
  - [ ] 绌?key 鎷掔粷
  - [ ] 妯″瀷鍥炴樉
  - [ ] 閰嶇疆浼樺厛绾?  - [ ] 闈炴硶妯″瀷鍚嶅鐞?- Integration Cases:
  - [ ] `/login` 鍚?`/model` 鍙敤
- E2E Smoke:
  - [ ] 鐧诲綍鍚庨棶绛斿墠缃潯浠舵弧瓒?- Commands:
- Expected:
- Actual:
- Coverage:
  - Core:
  - Overall:
- Result:
- Notes:

### T09 - 鍗曡疆闂瓟闂幆
- Date:
- Env:
- Scope: one-turn chat path
- Unit Cases:
  - [ ] 杈撳叆鍒嗙被
  - [ ] 璇锋眰鏋勯€?  - [ ] 鍥炲娓叉煋
  - [ ] 绌哄洖澶嶉檷绾?  - [ ] 澶氳杈撳叆澶勭悊
  - [ ] 杈撳嚭鎴柇澶勭悊
- Integration Cases:
  - [ ] 鐧诲綍鍚庨棶绛旀垚鍔?- E2E Smoke:
  - [ ] `login -> ask -> answer`
- Commands:
- Expected:
- Actual:
- Coverage:
  - Core:
  - Overall:
- Result:
- Notes:

### T10 - 瓒呮椂涓庨敊璇槧灏?- Date:
- Env:
- Scope: timeout + 401/429/5xx handling
- Unit Cases:
  - [ ] 瓒呮椂瑙﹀彂
  - [ ] 401 鏄犲皠
  - [ ] 429 鏄犲皠
  - [ ] 5xx 鏄犲皠
  - [ ] 閲嶈瘯娆℃暟杈圭晫
  - [ ] 涓嶅彲閲嶈瘯鍒嗘敮
- Integration Cases:
  - [ ] 429 閲嶈瘯鍚庢仮澶?- E2E Smoke:
  - [ ] 寮傚父鎻愮ず鍙
- Commands:
- Expected:
- Actual:
- Coverage:
  - Core:
  - Overall:
- Result:
- Notes:

### T11 - SQLite 涓?migration v1
- Date:
- Env:
- Scope: db init and schema
- Unit Cases:
  - [ ] migration 骞傜瓑
  - [ ] 琛ㄧ粨鏋勫瓨鍦?  - [ ] 绱㈠紩瀛樺湪
  - [ ] 杩炴帴澶辫触澶勭悊
  - [ ] 鏉冮檺閿欒澶勭悊
  - [ ] 鏂囦欢閿佸鐞?- Integration Cases:
  - [ ] 棣栨鍚姩鑷姩寤哄簱
- E2E Smoke:
  - [ ] 鍚姩鍚?DB 鏂囦欢鍙敤
- Commands:
- Expected:
- Actual:
- Coverage:
  - Core:
  - Overall:
- Result:
- Notes:

### T12 - 娑堟伅鎸佷箙鍖栦粨鍌?- Date:
- Env:
- Scope: session/message repository
- Unit Cases:
  - [ ] 鏂板缓浼氳瘽
  - [ ] 鍐欐秷鎭?  - [ ] 鎺掑簭璇诲彇
  - [ ] 鍒嗛〉杈圭晫
  - [ ] 绌虹粨鏋滃鐞?  - [ ] 浜嬪姟鍥炴粴
- Integration Cases:
  - [ ] 鍐欏悗璇讳竴鑷?- E2E Smoke:
  - [ ] 閲嶅惎鍚庢秷鎭粛鍙鍙?- Commands:
- Expected:
- Actual:
- Coverage:
  - Core:
  - Overall:
- Result:
- Notes:

### T13 - /new
- Date:
- Env:
- Scope: create and switch session
- Unit Cases:
  - [ ] 榛樿鍛藉悕
  - [ ] 褰撳墠鎸囬拡鏇存柊
  - [ ] 閲嶅悕澶勭悊
  - [ ] 绌烘爣棰樺鐞?  - [ ] 鏃堕棿鎴崇敓鎴?  - [ ] 鍙傛暟瑙ｆ瀽
- Integration Cases:
  - [ ] `/new` 鍚庢秷鎭啓鍏ユ柊浼氳瘽
- E2E Smoke:
  - [ ] 鍒涘缓浼氳瘽鍚庡彲瀵硅瘽
- Commands:
- Expected:
- Actual:
- Coverage:
  - Core:
  - Overall:
- Result:
- Notes:

### T14 - /sessions
- Date:
- Env:
- Scope: list sessions
- Unit Cases:
  - [ ] 鎺掑簭瑙勫垯
  - [ ] 绌哄垪琛ㄦ彁绀?  - [ ] 褰撳墠浼氳瘽鏍囪
  - [ ] 鍒嗛〉鍙傛暟
  - [ ] 杈撳嚭鏍煎紡
  - [ ] 杩囨护鏉′欢
- Integration Cases:
  - [ ] 澶氫細璇濆睍绀烘纭?- E2E Smoke:
  - [ ] 浼氳瘽鍒楄〃鍙煡鐪?- Commands:
- Expected:
- Actual:
- Coverage:
  - Core:
  - Overall:
- Result:
- Notes:

### T15 - /switch
- Date:
- Env:
- Scope: switch by id/index
- Unit Cases:
  - [ ] id 鍒囨崲
  - [ ] index 鍒囨崲
  - [ ] 鐩爣涓嶅瓨鍦?  - [ ] 閲嶅鍒囨崲
  - [ ] 鍙傛暟缂哄け
  - [ ] 杈圭晫绱㈠紩
- Integration Cases:
  - [ ] 鍒囨崲鍚庡啓鍏ョ洰鏍囦細璇?- E2E Smoke:
  - [ ] 浼氳瘽鍒囨崲鍚庝笂涓嬫枃姝ｇ‘
- Commands:
- Expected:
- Actual:
- Coverage:
  - Core:
  - Overall:
- Result:
- Notes:

### T16 - /history
- Date:
- Env:
- Scope: history with limits
- Unit Cases:
  - [ ] 椤哄簭姝ｇ‘
  - [ ] limit 鐢熸晥
  - [ ] 绌哄巻鍙叉彁绀?  - [ ] 瑙掕壊鏄剧ず
  - [ ] 瓒呴暱鎴柇
  - [ ] 闈炴硶 limit
- Integration Cases:
  - [ ] 浼氳瘽鍒囨崲鍘嗗彶闅旂
- E2E Smoke:
  - [ ] 鍘嗗彶鏌ヨ鎴愬姛
- Commands:
- Expected:
- Actual:
- Coverage:
  - Core:
  - Overall:
- Result:
- Notes:

### T17 - 鍛戒护娉ㄥ唽涓績
- Date:
- Env:
- Scope: registry and metadata
- Unit Cases:
  - [ ] 娉ㄥ唽鎴愬姛
  - [ ] 鍒悕瑙ｆ瀽
  - [ ] 閲嶅悕鍐茬獊
  - [ ] 鏌ユ棤鍛戒护
  - [ ] 鎵归噺娉ㄥ唽
  - [ ] 椤哄簭绋冲畾
- Integration Cases:
  - [ ] 璺敱鍣ㄦ寜娉ㄥ唽鎵ц
- E2E Smoke:
  - [ ] 鍛戒护琛ㄥ彲灞曠ず
- Commands:
- Expected:
- Actual:
- Coverage:
  - Core:
  - Overall:
- Result:
- Notes:

### T18 - / 琛ュ叏 v1
- Date:
- Env:
- Scope: prefix completion
- Unit Cases:
  - [ ] 鍓嶇紑鍖归厤
  - [ ] 绌哄墠缂€杩斿洖
  - [ ] 鏃犲尮閰嶅鐞?  - [ ] 鍒悕妫€绱?  - [ ] 鎺掑簭绋冲畾
  - [ ] 澶у皬鍐欑瓥鐣?- Integration Cases:
  - [ ] `/mo` 鍊欓€夊寘鍚?`/model`
- E2E Smoke:
  - [ ] 杈撳叆 `/` 鏄剧ず鍊欓€?- Commands:
- Expected:
- Actual:
- Coverage:
  - Core:
  - Overall:
- Result:
- Notes:

### T19 - Tab 鎺ュ彈琛ュ叏
- Date:
- Env:
- Scope: tab accept flow
- Unit Cases:
  - [ ] 鍗曞€欓€夋帴鍙?  - [ ] 澶氬€欓€夋帴鍙楀綋鍓嶉」
  - [ ] 鏃犲€欓€変繚鎸佷笉鍙?  - [ ] 鍏夋爣浣嶇疆淇濇寔
  - [ ] 灏鹃殢绌烘牸澶勭悊
  - [ ] 杩炵画 Tab 琛屼负
- Integration Cases:
  - [ ] `/mo + Tab -> /model`
- E2E Smoke:
  - [ ] 琛ュ叏鍚庡彲鎵ц鍛戒护
- Commands:
- Expected:
- Actual:
- Coverage:
  - Core:
  - Overall:
- Result:
- Notes:

### T20 - 鍊欓€夊鑸笌鍙栨秷
- Date:
- Env:
- Scope: arrow navigation and esc close
- Unit Cases:
  - [ ] 涓嬫柟鍚戝惊鐜?  - [ ] 涓婃柟鍚戝惊鐜?  - [ ] Esc 娓呯┖鍊欓€?  - [ ] 鏃犲€欓€夋椂鎸夐敭蹇界暐
  - [ ] 鐒︾偣鍒囨崲
  - [ ] 鐘舵€佸浣?- Integration Cases:
  - [ ] 瀵艰埅鍚?Tab 鍛戒腑姝ｇ‘椤?- E2E Smoke:
  - [ ] 瀵艰埅涓庡彇娑堟祦绋嬪彲鐢?- Commands:
- Expected:
- Actual:
- Coverage:
  - Core:
  - Overall:
- Result:
- Notes:

### T21 - 棰戞鎺掑簭
- Date:
- Env:
- Scope: usage frequency ranking
- Unit Cases:
  - [ ] 棰戞绱姞
  - [ ] 棣栨鍛戒护鍒濆鍖?  - [ ] 鍚岄绋冲畾鎺掑簭
  - [ ] 棰戞鎸佷箙鍖?  - [ ] 閲嶅惎鍚庢帓搴忎繚鐣?  - [ ] 涓婇檺淇濇姢
- Integration Cases:
  - [ ] 楂橀鍛戒护鍓嶇疆
- E2E Smoke:
  - [ ] 杩炵画浣跨敤鍚庤ˉ鍏ㄥ彉鍖?- Commands:
- Expected:
- Actual:
- Coverage:
  - Core:
  - Overall:
- Result:
- Notes:

### T22 - /run 鍙鎵ц鍣?- Date:
- Env:
- Scope: read-only run command
- Unit Cases:
  - [ ] 骞冲彴鍛戒护鎷兼帴
  - [ ] 鐧藉悕鍗曟牎楠?  - [ ] 杈撳嚭鎹曡幏
  - [ ] 閫€鍑虹爜鏄犲皠
  - [ ] stderr 澶勭悊
  - [ ] 闀胯緭鍑烘埅鏂?- Integration Cases:
  - [ ] 鎵ц鍙鍛戒护鎴愬姛
- E2E Smoke:
  - [ ] `/run` 鍙繑鍥炶緭鍑?- Commands:
- Expected:
- Actual:
- Coverage:
  - Core:
  - Overall:
- Result:
- Notes:

### T23 - 椋庨櫓鍒嗙骇鍣?- Date:
- Env:
- Scope: risk level classification
- Unit Cases:
  - [ ] low 鍒ゅ畾
  - [ ] medium 鍒ゅ畾
  - [ ] high 鍒ゅ畾
  - [ ] 娣峰悎鍛戒护鍒ゅ畾
  - [ ] 澶у皬鍐?绌虹櫧澶勭悊
  - [ ] 缁曡繃鍐欐硶鎷︽埅
- Integration Cases:
  - [ ] `/run` 鎺ュ叆椋庨櫓鍒嗙被
- E2E Smoke:
  - [ ] 楂橀闄╁懡浠よ矾寰勮璇嗗埆
- Commands:
- Expected:
- Actual:
- Coverage:
  - Core:
  - Overall:
- Result:
- Notes:

### T24 - 纭娴佺▼
- Date:
- Env:
- Scope: medium/high approval gate
- Unit Cases:
  - [ ] 纭閫氳繃鎵ц
  - [ ] 纭鎷掔粷闃绘柇
  - [ ] 瓒呮椂榛樿鎷掔粷
  - [ ] 闈炴硶杈撳叆閲嶈瘯
  - [ ] high 椋庨櫓鎻愮ず
  - [ ] 鐘舵€佸洖鏀?- Integration Cases:
  - [ ] 鏈‘璁や笉鑳芥墽琛岄珮椋庨櫓
- E2E Smoke:
  - [ ] `reject -> no execution`
- Commands:
- Expected:
- Actual:
- Coverage:
  - Core:
  - Overall:
- Result:
- Notes:

### T25 - 瀹¤璁板綍
- Date:
- Env:
- Scope: audit trail and query
- Unit Cases:
  - [ ] 瀹¤鍏ュ簱
  - [ ] 瀹℃壒鐘舵€佽褰?  - [ ] 缁撴灉瀛楁璁板綍
  - [ ] 鏃堕棿鎴虫牸寮?  - [ ] 鏌ヨ杩囨护
  - [ ] 鍒嗛〉
- Integration Cases:
  - [ ] `/history --audit` 鍙煡璇?- E2E Smoke:
  - [ ] 鎵ц鍚庡璁″彲瑙?- Commands:
- Expected:
- Actual:
- Coverage:
  - Core:
  - Overall:
- Result:
- Notes:

### T26 - /add
- Date:
- Env:
- Scope: add context files
- Unit Cases:
  - [ ] 璺緞瑙勮寖鍖?  - [ ] 閲嶅鍘婚噸
  - [ ] 鏂囦欢涓嶅瓨鍦?  - [ ] 鐩綍杈撳叆澶勭悊
  - [ ] 浜岃繘鍒舵帓闄?  - [ ] 缂栫爜寮傚父澶勭悊
- Integration Cases:
  - [ ] 娣诲姞鍚庡垪琛ㄥ彲瑙?- E2E Smoke:
  - [ ] 娣诲姞鍚庢彁闂彲寮曠敤涓婁笅鏂?- Commands:
- Expected:
- Actual:
- Coverage:
  - Core:
  - Overall:
- Result:
- Notes:

### T27 - /drop
- Date:
- Env:
- Scope: remove context files
- Unit Cases:
  - [ ] 鎸夎矾寰勭Щ闄?  - [ ] 鎸夌储寮曠Щ闄?  - [ ] 涓嶅瓨鍦ㄩ」澶勭悊
  - [ ] 绌洪泦鍚堝鐞?  - [ ] 鎵归噺绉婚櫎
  - [ ] 鎺掑簭淇濇寔
- Integration Cases:
  - [ ] 绉婚櫎鍚庤姹備笉鍚鏂囦欢
- E2E Smoke:
  - [ ] 绉婚櫎鍚庝笂涓嬫枃鏇存柊
- Commands:
- Expected:
- Actual:
- Coverage:
  - Core:
  - Overall:
- Result:
- Notes:

### T28 - /files
- Date:
- Env:
- Scope: list context files
- Unit Cases:
  - [ ] 绌哄垪琛ㄦ彁绀?  - [ ] 鎺掑簭瑙勫垯
  - [ ] 璺緞缂╃暐鏄剧ず
  - [ ] 鏉℃暟闄愬埗
  - [ ] 杩囨护瑙勫垯
  - [ ] 杈撳嚭鏍煎紡
- Integration Cases:
  - [ ] `/add` 鍚?`/files` 姝ｇ‘
- E2E Smoke:
  - [ ] 鍒楄〃鍙鍖栬緭鍑?- Commands:
- Expected:
- Actual:
- Coverage:
  - Core:
  - Overall:
- Result:
- Notes:

### T29 - /grep
- Date:
- Env:
- Scope: read-only grep
- Unit Cases:
  - [ ] pattern 鏍￠獙
  - [ ] ignore 鐢熸晥
  - [ ] 琛屽彿鏍煎紡
  - [ ] 绌虹粨鏋滄彁绀?  - [ ] 闄愭祦澶勭悊
  - [ ] 闈炴硶姝ｅ垯澶勭悊
- Integration Cases:
  - [ ] 椤圭洰鐩綍妫€绱㈡垚鍔?- E2E Smoke:
  - [ ] 妫€绱㈢粨鏋滃彲杩斿洖
- Commands:
- Expected:
- Actual:
- Coverage:
  - Core:
  - Overall:
- Result:
- Notes:

### T30 - /tree
- Date:
- Env:
- Scope: project tree browsing
- Unit Cases:
  - [ ] 鏍戞瀯寤?  - [ ] 娣卞害瑁佸壀
  - [ ] ignore 瑙勫垯
  - [ ] 绗﹀彿閾炬帴澶勭悊
  - [ ] 鏉冮檺閿欒澶勭悊
  - [ ] 杈撳嚭鏍煎紡绋冲畾
- Integration Cases:
  - [ ] 鎸囧畾鐩綍杈撳嚭姝ｇ‘
- E2E Smoke:
  - [ ] tree 鍛戒护鍙繍琛?- Commands:
- Expected:
- Actual:
- Coverage:
  - Core:
  - Overall:
- Result:
- Notes:

### T31 - 涓婁笅鏂囩粍瑁呭櫒
- Date:
- Env:
- Scope: context assembly to prompt
- Unit Cases:
  - [ ] 鎷兼帴椤哄簭
  - [ ] 瑙掕壊鏍囪
  - [ ] 閲嶅鍘婚噸
  - [ ] 绌轰笂涓嬫枃鐩撮€?  - [ ] 鍏冧俊鎭～鍏?  - [ ] 缂栫爜缁熶竴
- Integration Cases:
  - [ ] 璇锋眰杞借嵎鍖呭惈涓婁笅鏂?- E2E Smoke:
  - [ ] 涓婁笅鏂囬棶绛旇矾寰勬垚鍔?- Commands:
- Expected:
- Actual:
- Coverage:
  - Core:
  - Overall:
- Result:
- Notes:

### T32 - Token 棰勭畻瑁佸壀
- Date:
- Env:
- Scope: token budget and trimming
- Unit Cases:
  - [ ] 棰勭畻璁＄畻
  - [ ] 杈圭晫绛変簬棰勭畻
  - [ ] 瓒呴绠楁埅鏂?  - [ ] 浼樺厛绾т繚鐣?  - [ ] 鏋佸皬棰勭畻闄嶇骇
  - [ ] 鎻愮ず鏂囨
- Integration Cases:
  - [ ] 闀夸笂涓嬫枃璇锋眰涓嶈秴闄?- E2E Smoke:
  - [ ] 瓒呴檺鏃朵粛鍙畬鎴愯姹?- Commands:
- Expected:
- Actual:
- Coverage:
  - Core:
  - Overall:
- Result:
- Notes:

### T33 - JSON Schema 鍛戒护娉ㄥ唽
- Date:
- Env:
- Scope: schema-driven registration
- Unit Cases:
  - [ ] schema 閫氳繃
  - [ ] 缂哄瓧娈垫姤閿?  - [ ] 鍙傛暟绫诲瀷鏍￠獙
  - [ ] handler 缂哄け鎶ラ敊
  - [ ] alias 鍐茬獊鎶ラ敊
  - [ ] examples 瑙ｆ瀽
- Integration Cases:
  - [ ] schema 娉ㄥ唽鍚庡懡浠ゅ彲鎵ц
- E2E Smoke:
  - [ ] schema 鍛戒护鍑虹幇鍦?help
- Commands:
- Expected:
- Actual:
- Coverage:
  - Core:
  - Overall:
- Result:
- Notes:

### T34 - 鍛戒护鎵╁睍鍒?18 鏉?- Date:
- Env:
- Scope: expand command set to 18
- Unit Cases:
  - [ ] 鍙傛暟瑙ｆ瀽
  - [ ] 甯姪鏂囨
  - [ ] 鏉冮檺鏍囪
  - [ ] 榛樿鍙傛暟
  - [ ] 閿欒鎻愮ず
  - [ ] 鍒悕琛屼负
- Integration Cases:
  - [ ] 18 鏉″懡浠ゅ彲鏋氫妇鍙墽琛?- E2E Smoke:
  - [ ] 澶氬懡浠よ矾寰勫彲杩愯
- Commands:
- Expected:
- Actual:
- Coverage:
  - Core:
  - Overall:
- Result:
- Notes:

### T35 - 鎵╁睍鍒?27 鏉?+ 鍙傛暟琛ュ叏
- Date:
- Env:
- Scope: full command set + arg completion
- Unit Cases:
  - [ ] 鍙傛暟鍊欓€夌敓鎴?  - [ ] 涓婁笅鏂囩浉鍏冲€欓€?  - [ ] 鏃犳晥鍙傛暟杩囨护
  - [ ] 瀛愬懡浠よˉ鍏?  - [ ] 琛ュ叏涓庢墽琛屼竴鑷?  - [ ] 鍏夋爣浣嶇疆澶勭悊
- Integration Cases:
  - [ ] `/config set` 鍙傛暟琛ュ叏
- E2E Smoke:
  - [ ] 鍛戒护涓庡弬鏁拌ˉ鍏ㄥ叏閾捐矾
- Commands:
- Expected:
- Actual:
- Coverage:
  - Core:
  - Overall:
- Result:
- Notes:

### T36 - /config
- Date:
- Env:
- Scope: read and set config
- Unit Cases:
  - [ ] 璇诲彇閰嶇疆閿?  - [ ] 璁剧疆閰嶇疆閿?  - [ ] 绫诲瀷杞崲
  - [ ] 鍙閿繚鎶?  - [ ] 鏈煡閿姤閿?  - [ ] 鍐欏洖鎸佷箙鍖?- Integration Cases:
  - [ ] 閲嶅惎鍚庨厤缃敓鏁?- E2E Smoke:
  - [ ] 鍔ㄦ€佷慨鏀归厤缃矾寰勫彲鐢?- Commands:
- Expected:
- Actual:
- Coverage:
  - Core:
  - Overall:
- Result:
- Notes:

### T37 - /export
- Date:
- Env:
- Scope: export session to json/md
- Unit Cases:
  - [ ] 瀵煎嚭鏍煎紡鐢熸垚
  - [ ] 鏂囦欢鍚嶈鑼?  - [ ] 绌轰細璇濆鐞?  - [ ] 鏉冮檺閿欒澶勭悊
  - [ ] 瑕嗙洊绛栫暐
  - [ ] 缂栫爜涓€鑷存€?- Integration Cases:
  - [ ] 瀵煎嚭鏂囦欢鍙鍙?- E2E Smoke:
  - [ ] 瀵煎嚭鍚庢枃浠跺瓨鍦ㄤ笖鍐呭姝ｇ‘
- Commands:
- Expected:
- Actual:
- Coverage:
  - Core:
  - Overall:
- Result:
- Notes:

### T38 - Shell 缁熶竴閫傞厤灞?- Date:
- Env:
- Scope: cross-platform shell adapter
- Unit Cases:
  - [ ] 骞冲彴妫€娴?  - [ ] 鍛戒护杞箟
  - [ ] 鐜鍙橀噺娉ㄥ叆
  - [ ] 宸ヤ綔鐩綍鍒囨崲
  - [ ] 瓒呮椂缁堟
  - [ ] 淇″彿澶勭悊
- Integration Cases:
  - [ ] Win/Linux 閫傞厤琛屼负涓€鑷?- E2E Smoke:
  - [ ] 璺ㄥ钩鍙板懡浠ゆ墽琛岄€氳繃
- Commands:
- Expected:
- Actual:
- Coverage:
  - Core:
  - Overall:
- Result:
- Notes:

### T39 - 鎵撳寘鍙戝竷浜х墿
- Date:
- Env:
- Scope: build distributable binaries
- Unit Cases:
  - [ ] 鏋勫缓閰嶇疆瑙ｆ瀽
  - [ ] 鐗堟湰鍙锋敞鍏?  - [ ] 浜х墿鍛藉悕瑙勫垯
  - [ ] 缂轰緷璧栨姤閿?  - [ ] 鍏ュ彛鏍￠獙
  - [ ] 鍙傛暟杈圭晫
- Integration Cases:
  - [ ] 浜х墿 `--version` 鍙墽琛?- E2E Smoke:
  - [ ] 涓嬭浇鍚庡惎鍔?smoke 鎴愬姛
- Commands:
- Expected:
- Actual:
- Coverage:
  - Core:
  - Overall:
- Result:
- Notes:

### T40 - CI 璐ㄩ噺闂ㄧ
- Date:
- Env:
- Scope: CI pipeline and quality gates
- Unit Cases:
  - [ ] CI 閰嶇疆璇硶
  - [ ] 瑕嗙洊鐜囬槇鍊肩瓥鐣?  - [ ] 浠诲姟椤哄簭渚濊禆
  - [ ] 澶辫触閲嶈瘯绛栫暐
  - [ ] 缂撳瓨绛栫暐
  - [ ] 宸ヤ欢褰掓。绛栫暐
- Integration Cases:
  - [ ] 妯℃嫙 PR 娴佺▼閫氳繃
- E2E Smoke:
  - [ ] 榛勯噾璺緞鍦?CI 鎴愬姛
- Commands:
- Expected:
- Actual:
- Coverage:
  - Core:
  - Overall:
- Result:
- Notes:

