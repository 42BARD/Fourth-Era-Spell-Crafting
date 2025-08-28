;BEGIN FRAGMENT CODE - Do not edit anything between this and the end comment
;NEXT FRAGMENT INDEX 1
Scriptname JR12_TIF__0100A999 Extends TopicInfo Hidden

;BEGIN FRAGMENT Fragment_0
Function Fragment_0(ObjectReference akSpeakerRef)
Actor akSpeaker = akSpeakerRef as Actor
;BEGIN CODE
Game.GetPlayer().RemoveItem(Gold, TomeCost.GetValueInt())

if GetOwningQuest().GetStage() == 5
GetOwningQuest().SetStage(20)
Else
GetOwningQuest().SetStage(10)
Endif

Game.GetPlayer().AddItem(SCTome as form, 1)
;END CODE
EndFunction
;END FRAGMENT

;END FRAGMENT CODE - Do not edit anything between this and the begin comment

MiscObject Property Gold  Auto  

GlobalVariable Property TomeCost  Auto  

Book Property SCTome  Auto  
