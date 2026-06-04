
#ifndef SIP_TRANSACTION_H
#define SIP_TRANSACTION_H

#include "sip_message.h"
#include <stdexcept>
#include <string>

enum class SipTransactionState
{
    Idle, //空闲：初始状态，等待发送请求或接收请求
    Calling, //等待1：如果对方不响应，UAC超时重传。
    Proceeding, //等待2：如果对方回复了100/180后，不再重传，继续等。
    Connected, //通话中：如果对方回复了200OK后，回复ACK后 进入通话中状态
    Terminated //结束：在等待过程中，UAC主动发CANCEL 取消外呼，对方回复200OK后走到终态
               //结束： 对方回复486/603 表示 拒绝，走到终态 
};

enum class SipTransactionEvent
{
    SendInvite, //发送INVITE，表示用户发起呼叫了，要走Calling分支
    Received100Or180, //收到100，180：表示对方活着，继续等待，不再重传INVITE
    Received183, //收到183，表示对方具备接听条件了，可以播放早期媒体
    Received200OK, //收到200OK，表示对方接听，这时要在有条件的前提下给对方一个确认表示成功通话。
    Received4xx5xx6xx, //收到4XX/5XX/6XX，表示呼叫失败，具体原因需要从code中解析，例如找不到路由/被叫/拒绝等。这时要走终止分支
    ReceivedBYE, //收到BYE 时要响应200OK，要走终止分支。
    SendCancel, //发送CANCEL，表示用户取消呼叫了，要走终止分支。
    SendBYE, //发送BYE，表示用户挂电话了，要走终止分支。
    TimerBTimeout //超时：Timer B 超时
};

/**
 * 
 * 
 * ：状态转移函数。

  签名是这样的：

  SipTransactionState transition(SipTransactionState current, SipTransactionEvent event);

  输入当前状态 + 发生的事件，输出下一个状态。非法的组合（比如 Idle 状态收到 ReceivedBYE）抛异常。
 

  当前状态 + 事件 → 下一状态
 

    idle + SendInvite → Calling

    Calling + Received100Or180 → Proceeding
    Calling + Received183 → Proceeding

    Calling + Received200OK → Connected  // UAS 可以不发任何 1xx，直接回 200 OK——电话接通极快时会发生这种情况。Calling 状态下也必须处理 200 OK。
    Calling + TimerBTimeout → Terminated
    Calling + SendCancel → Terminated    
    Calling + Received4xx5xx6xx → Terminated

    Proceeding + Received4xx5xx6xx → Terminated
    Proceeding + Received200OK → Connected
    Proceeding + SendCancel → Terminated
    Proceeding + TimerBTimeout → Terminated
    Processding + Receive100Or180 → Proceeding  // 继续等，收到100/180后不再重传INVITE了
    Processing + Received183 → Proceeding  // 继续等，收到183后不再重传INVITE了

    Connected + ReceivedBYE → Terminated
    Connected + SendBYE → Terminated  //  UAC 自己挂电话是最常见的场景，这条转移不能省
 */
SipTransactionState transition(SipTransactionState current, SipTransactionEvent event);
#endif // SIP_TRANSACTION_H