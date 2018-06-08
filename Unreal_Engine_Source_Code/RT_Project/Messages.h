#pragma once

// Display on the screen a message in red
FORCEINLINE void ScreenMsg(const FString& Msg){
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, *Msg);
}
FORCEINLINE void ScreenMsg(const FString& Msg, const float Value){
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("%s %3.2f"), *Msg, Value));
}
FORCEINLINE void ScreenMsg(const FString& Msg, const int32 Value){
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("%s %d"), *Msg, Value));
}
FORCEINLINE void ScreenMsg(const FString& Msg, const FString& Msg2){
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("%s %s"), *Msg, *Msg2));
}
