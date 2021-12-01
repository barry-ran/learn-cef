# 在Cef中实现C++与JavaScript交互场景分析

本文主要介绍 CEF 场景中 C++ 和 JavaScript 交互（以下简称 JS Bridge）中的一些重要节点，包括了 C++/JavaScript 的方法注册、方法调用、回调管理。以下是一些重要的参考资料：
- CEF Util 中文翻译资料：https://github.com/fanfeilong/cefutil
- CEF 经典N大问题：https://blog.csdn.net/weolar/article/details/51994895
- CefV8Value to JSON：https://magpcss.org/ceforum/viewtopic.php?f=7&t=12774
- C++ 和 JS 交互：https://www.smwenku.com/a/5b8afe442b71773b27ca3b11/
# 现有实现的不足
在制作新的 JS Bridge 之前，团队中已经有将 Cef 整合到项目中的离屏渲染实现，但 C++ 与 JavaScript 交互的代码相对单一，仅实现了一些简单的方法，没有拓展性和统一性。也没有处理一些多 Render 和多 Browser 实例的情况。比如我希望调用一个 C++ 的方法，需要重新在 Render 和 Browser 进程中实现单独的通信代码，这样是非常麻烦的而且容易出错。

# 期望的样子
因为未来有跨平台的打算，所以侧重点还是往偏前端一些，希望所有界面展示的功能均交由前端来实现。所以首先前端可以很方便的提供接口让 C++ 调用，并且可以很方便的调用一个 C++ 接口并得到适当的回调返回信息。同理 C++ 端也希望能很容易的调用前端的方法或注册方法提供前端调用。它们之间传递数据使用通用的 JSON 格式，在 C++ 端总是以字符串方式解析，而在前端总是以一个 Object 的方式解析。因为 JSON 的拓展性极高，当做桥梁之间传递数据的通道最合适不过了。

# 前端调用 C++ 方法的流程
Render 进程的 OnWebKitInitialized 接口在 WebKit 初始化完成后被调用，此时我们可以通过 CefRegisterExtension 来注册一个拓展让 WebKit 初始化完成后就执行这部分代码，而这部分代码就完全靠你发挥了，你可以声明一个全局的对象，给该对象实现两个方法来提供前端页面注册方法和调用 Native 的方法，如下所示：
```
void ClientApp::OnWebKitInitialized() 
{
    /**
     * JavaScript 扩展代码，这里定义一个 NimCefWebFunction 对象提供 call 和 register 方法来让 Web 端触发 CefV8Handler 处理代码
     * param[in] functionName 要调用的 C++ 方法名称
     * param[in] params 调用该方法传递的参数，在前端指定的是一个 Object，但转到 Native 的时候转为了字符串
     * param[in] callback 执行该方法后的回调函数
     * 前端调用示例
     * NimCefWebHelper.call('showMessage', { message: 'Hello C++' }, (arguments) => {
     *    console.log(arguments)
     * })
     */
    std::string extensionCode = R"(
        var NimCefWebInstance = {};
        (() => {
            NimCefWebInstance.call = (functionName, arg1, arg2) => {
                if (typeof arg1 === 'function') {
                    native function call(functionName, arg1);
                    return call(functionName, arg1);
                } else {
                    const jsonString = JSON.stringify(arg1);
                    native function call(functionName, jsonString, arg2);
                    return call(functionName, jsonString, arg2);
                }
            };
            NimCefWebInstance.register = (functionName, callback) => {
                native function register(functionName, callback);
                return register(functionName, callback);
            };
        })();
    )";
    CefRefPtr<CefJSHandler> handler = new CefJSHandler();

     CefRegisterExtension("v8/extern", extensionCode, handler);
}
```
代码中新增了一个 NimCefWebInstance 全局对象，并拓展了一个 call 方法和一个 register 方法，分别提供前端调用 C++ 方法和注册本地的方法让 C++ 调用。并且做了适当判断，允许传递参数和不传递参数。如果你更了解 JavaScript 可以进一步拓展。

另外可以看到我们新建了一个派生于 CefV8Handler 类的 CefJSHandler 类，该类仅实现了一个方法，就是用来接收我们刚才注册到页面中的方法事件的。实现如下：
```
bool CefJSHandler::Execute(const CefString& name, CefRefPtr<CefV8Value> object, const CefV8ValueList& arguments, CefRefPtr<CefV8Value>& retval, CefString& exception)
{
    // 当Web中调用了"NimCefWebFunction"函数后，会触发到这里，然后把参数保存，转发到Broswer进程
    // Broswer进程的BrowserHandler类在OnProcessMessageReceived接口中处理kJsCallbackMessage消息，就可以收到这个消息

    if (arguments.size() < 2)
    {
        exception = "Invalid arguments.";
        return false;
    }

    CefRefPtr<CefV8Context> context = CefV8Context::GetCurrentContext();
    CefRefPtr<CefFrame> frame = context->GetFrame();
    CefRefPtr<CefBrowser> browser = context->GetBrowser();

    int64_t browser_id = browser->GetIdentifier();
    int64_t frame_id = frame->GetIdentifier();

    if (name == "call")
    {
        // 允许没有参数列表的调用，第二个参数为回调
        // 如果传递了参数列表，那么回调是第三个参数
        CefString function_name = arguments[0]->GetStringValue();
        CefString params = "{}";
        CefRefPtr<CefV8Value> callback;
        if (arguments[0]->IsString() && arguments[1]->IsFunction())
        {
            callback = arguments[1];
        }
        else if (arguments[0]->IsString() && arguments[1]->IsString() && arguments[2]->IsFunction())
        {
            params = arguments[1]->GetStringValue();
            callback = arguments[2];
        }
        else
        {
            exception = "Invalid arguments.";
            return false;
        }

        // 执行 C++ 方法
        if (!js_bridge_->CallCppFunction(function_name, params, callback))
        {
            exception = nbase::StringPrintf("Failed to call function %s.", function_name).c_str();
            return false;
        }

        return true;
    }
    else if (name == "register")
    {
        if (arguments[0]->IsString() && arguments[1]->IsFunction())
        {
            std::string function_name = arguments[0]->GetStringValue();
            CefRefPtr<CefV8Value> callback = arguments[1];
            if (!js_bridge_->RegisterJSFunc(function_name, callback))
            {
                exception = "Failed to register function.";
                return false;
            }
            return true;
        }
        else
        {
            exception = "Invalid arguments.";
            return false;
        }
    }

    return false;
}
```
这里我们区分了 call 和 register 方法，并且进一步判断了参数的传递顺序。当前端执行了 call 方法时就可以将执行的函数名、传递参数保存下来，然后通知 Browser 进程去执行这个方法（前提是 Browser 端已经注册过使用相同字符串命名的这个方法）。我将该操作传递给了一个 js_bridge 对象的 CallCppFunction 方法。这是我封装的一个用来管理两端注册的方法和回调的管理类，并将两端通讯的方法封装了起来，如下所示：
```
bool CefJSBridge::CallCppFunction(const CefString& function_name, const CefString& params, CefRefPtr<CefV8Value> callback)
{
    auto it = render_callback_.find(js_callback_id_);
    if (it == render_callback_.cend())
    {
        CefRefPtr<CefV8Context> context = CefV8Context::GetCurrentContext();
        CefRefPtr<CefProcessMessage> message = CefProcessMessage::Create(kCallCppFunctionMessage);

        message->GetArgumentList()->SetString(0, function_name);
        message->GetArgumentList()->SetString(1, params);
        message->GetArgumentList()->SetInt(2, js_callback_id_);

        render_callback_.emplace(js_callback_id_++, std::make_pair(context, callback));

        // 发送消息到 browser 进程
        CefRefPtr<CefBrowser> browser = context->GetBrowser();
        browser->SendProcessMessage(PID_BROWSER, message);

        return true;
    }

    return false;
}
```
这里我们维护了一份 callback 的索引，每当发起新的调用时，这个索引值自增，并插入到我们管理回调的 map 结构中。map 中以 callback 索引为标准，存储了运行环境和真正的 callback 实体。最后使用 SendProcessMessage方法通知 Browser 来执行我们要运行的代码。当消息发出后，Browser 进程就会收到这个消息了。
```
bool BrowserHandler::OnProcessMessageReceived(CefRefPtr<CefBrowser> browser, CefProcessId source_process, CefRefPtr<CefProcessMessage> message)
{
    // 处理render进程发来的消息
    std::string message_name = message->GetName();
    if (message_name == kFocusedNodeChangedMessage)
    {
        is_focus_oneditable_field_ = message->GetArgumentList()->GetBool(0);
        return true;
    }
    else if (message_name == kCallCppFunctionMessage)
    {
        CefString fun_name    = message->GetArgumentList()->GetString(0);
        CefString param        = message->GetArgumentList()->GetString(1);
        int js_callback_id    = message->GetArgumentList()->GetInt(2);

        if (handle_delegate_)
            handle_delegate_->OnExecuteCppFunc(fun_name, param, js_callback_id, browser);

        return true;
    }
    else if (message_name == kExecuteCppCallbackMessage)
    {
        CefString param = message->GetArgumentList()->GetString(0);
        int callback_id = message->GetArgumentList()->GetInt(1);

        if (handle_delegate_)
            handle_delegate_->OnExecuteCppCallbackFunc(callback_id, param);
    }

    return false;
}
```
Browser 进程接收到消息后，判断如果是 kCallCppFunctionMessage 消息类型那么就将要执行的函数名和参数传递给一个委托类去做具体的执行。实际委托类的子类中实现了这些执行 C++ 方法的虚函数，在实现的虚函数中解析了参数和要调用的函数名，通过 js_bridge 对象来执行曾经注册过的方法。当 C++ 方法执行完以后，我们还要通知 Render 进程去执行回调函数，如下所示：
```
bool CefJSBridge::ExecuteCppFunc(const CefString& function_name, const CefString& params, int js_callback_id, CefRefPtr<CefBrowser> browser)
{
    CefRefPtr<CefProcessMessage> message = CefProcessMessage::Create(kExecuteJsCallbackMessage);
    CefRefPtr<CefListValue> args = message->GetArgumentList();

    auto it = browser_registered_function_.find(std::make_pair(function_name, browser->GetIdentifier()));
    if (it != browser_registered_function_.cend())
    {
        auto function = it->second;
        Post2UI([=]() {
            function(params, [=](bool has_error, const std::string& json_result) {
                // 测试代码，需要封装到管理器中
                args->SetInt(0, js_callback_id);
                args->SetBool(1, has_error);
                args->SetString(2, json_result);
                browser->SendProcessMessage(PID_RENDERER, message);
            });
        });
        return true;
    }
    else
    {
        args->SetInt(0, js_callback_id);
        args->SetBool(1, true);
        args->SetString(2, R"({"message":"Function does not exist."})");
        browser->SendProcessMessage(PID_RENDERER, message);
        return false;
    }
}
```
通过 SendProcessMessage 通知 Render 进程，我们要执行某个 Id 的 callback。当 Render 进程接收到这个消息后，会根据传递进来的 callback id 去 map 中寻找这个 callback 的运行环境和实体来执行 callback 并传入 Browser 进程携带过来的参数。

# C++ 调用前端方法流程
还记得上面提到的全局方法中有个 register 方法吗？这个方法提供了前端注册持久化的方法提供 C++ 调用。注册的方法如下所示：
```
(() => {
    /*
     * 注册一个回调函数，用于在 C++ 应用中调用
     * param[in] showJsMessage 回调函数的名称，C++ 会使用该名称来调用此回调函数
     * param[in] callback 回调函数执行体
     */
    NimCefWebInstance.register('showJsMessage', (arguments) => {
        const receiveMessageInput = document.getElementById('receive_message_input')
        receiveMessageInput.value = arguments.message
        return {
            message: 'showJsMessage function was executed, this message return by JavaScript.'
        }
    })
})()
```
同样，在执行 register 方法注册一个持久化方法时会进入到上面提到的我们自己注册的 Handler::Execute 方法中。在一系列判断后开始将注册的函数放到 JS Bridge 维护的列表中，代码如下：
```
bool CefJSBridge::RegisterJSFunc(const CefString& function_name, CefRefPtr<CefV8Value> function, bool replace/* = false*/)
{
    CefRefPtr<CefV8Context> context = CefV8Context::GetCurrentContext();
    CefRefPtr<CefFrame> frame = context->GetFrame();

    if (replace)
    {
        render_registered_function_.emplace(std::make_pair(function_name, frame->GetIdentifier()), function);
        return true;
    }
    else
    {
        auto it = render_registered_function_.find(std::make_pair(function_name, frame->GetIdentifier()));
        if (it == render_registered_function_.cend())
        {
            render_registered_function_.emplace(std::make_pair(function_name, frame->GetIdentifier()), function);
            return true;
        }

        return false;
    }

    return false;
}
```
存放这些持久化函数时，我们根据函数名和当前注册函数所在的 frame id 为标准，为什么要加一个 frame id 呢？主要我们要考虑的是如果一个页面下存在多个 frame，不同的 frame 我们要允许他们注册同名的方法，在调用的时候去调用对应 frame 中的方法。另外一种情况就是如果你的 JS Bridge 是一个单例，它维护了所有 render 进程的所有 browser 实例的函数和回调列表，我们一样还是要用一个唯一的数据来区分某个 callback 要在哪个 frame 里执行。frame 是最小单位，并且在我实战情况下不同的 browser 下的 frame id 是不会重复的。所以用 frame id 做一个唯一标识是最靠谱的。 当 C++ 要调用前端已经注册好的方法时，只需要到这个列表中根据名字和 frame id 找到对应的 frame，通过 frame 得到运行上下文（context），然后进入这个上下文执行环境执行具体的函数体就可以啦。代码如下：
```
bool CefJSBridge::ExecuteJSFunc(const CefString& function_name, const CefString& json_params, CefRefPtr<CefFrame> frame, int cpp_callback_id)
{
    auto it = render_registered_function_.find(std::make_pair(function_name, frame->GetIdentifier()));
    if (it != render_registered_function_.cend())
    {

        auto context = frame->GetV8Context();
        auto function = it->second;

        if (context.get() && function.get())
        {
            context->Enter();

            CefV8ValueList arguments;

            // 将 C++ 传递过来的 JSON 转换成 Object
            CefV8ValueList json_parse_args;
            json_parse_args.push_back(CefV8Value::CreateString(json_params));
            CefRefPtr<CefV8Value> json_object = context->GetGlobal()->GetValue("JSON");
            CefRefPtr<CefV8Value> json_parse = json_object->GetValue("parse");
            CefRefPtr<CefV8Value> json_stringify = json_object->GetValue("stringify");
            CefRefPtr<CefV8Value> json_object_args = json_parse->ExecuteFunction(NULL, json_parse_args);
            arguments.push_back(json_object_args);

            // 执行回调函数
            CefRefPtr<CefV8Value> retval = function->ExecuteFunction(NULL, arguments);
            if (retval.get() && retval->IsObject())
            {
                // 回复调用 JS 后的返回值
                CefV8ValueList json_stringify_args;
                json_stringify_args.push_back(retval);
                CefRefPtr<CefV8Value> json_string = json_stringify->ExecuteFunction(NULL, json_stringify_args);
                CefString str = json_string->GetStringValue();

                CefRefPtr<CefProcessMessage> message = CefProcessMessage::Create(kExecuteCppCallbackMessage);
                CefRefPtr<CefListValue> args = message->GetArgumentList();
                args->SetString(0, json_string->GetStringValue());
                args->SetInt(1, cpp_callback_id);
                context->GetBrowser()->SendProcessMessage(PID_RENDERER, message);
            }

            context->Exit();

            return true;
        }

        return false;
    }

    return false;
}
```
这样前端应用就可以正常执行已经注册过的函数了。另外在上面的代码中，我们看到 ExecuteFunction 方法是又返回值的，这个返回值是前端 return 的数据。我们可以使用这个返回值再来通知 C++ 端执行的结果，我这里直接将执行结果通过进程间通信发送给了 C++ 端，虽然与前端调用 C++ 的回调实现不太一样，但是还是可以达到我们的需求的。

# 总结
上面分别介绍了两端互相注册和调用对端方法的示例，实际情况还是自己要根据项目需求设计一下，上面的实现思路还是有一些缺陷的。比如调用函数使用的是字符串名字，这样是挺不靠谱的做法，但从目前情况来看是最方便快捷的实现方式。但最终还是期望后期可以拓展成以对象方式直接调用对端方法，这可能要再对 Cef 做挖掘，根据自己实际项目情况再继续拓展了。