#include "sip_transaction.h"


SipTransactionState transition(SipTransactionState current, SipTransactionEvent event){
    switch (current)
    {
    case SipTransactionState::Idle:
        if (event == SipTransactionEvent::SendInvite){
            return SipTransactionState::Calling;
        }
        break;
    case SipTransactionState::Calling:
        if (event == SipTransactionEvent::Received100Or180 || event == SipTransactionEvent::Received183){
            return SipTransactionState::Proceeding;
        }else if (event == SipTransactionEvent::Received200OK){
            return SipTransactionState::Connected;
        }else if (event == SipTransactionEvent::Received4xx5xx6xx || event == SipTransactionEvent::SendCancel || event == SipTransactionEvent::TimerBTimeout){
            return SipTransactionState::Terminated;
        }
        break;
    case SipTransactionState::Proceeding:
        if (event == SipTransactionEvent::Received200OK){
            return SipTransactionState::Connected;
        }else if (event == SipTransactionEvent::Received4xx5xx6xx || event == SipTransactionEvent::SendCancel || event == SipTransactionEvent::TimerBTimeout){
            return SipTransactionState::Terminated;
        }else if (event == SipTransactionEvent::Received100Or180 || event == SipTransactionEvent::Received183){
            return SipTransactionState::Proceeding; // 继续等，收到100/180/183后不再重传INVITE了
        }

        break;
    case SipTransactionState::Connected:
        if (event == SipTransactionEvent::ReceivedBYE || event == SipTransactionEvent::SendBYE){
            return SipTransactionState::Terminated;
        }
        break;
    default:
        break;
    }
    throw std::invalid_argument("Invalid state transition: " + std::to_string(static_cast<int>(current)) + " + " + std::to_string(static_cast<int>(event)));
}
