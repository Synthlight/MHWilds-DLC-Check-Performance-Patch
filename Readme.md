This is a patch on `app.dlc.DlcService::isAvailableDLC(app.dlc.DlcProductId.ID)` to cache the results.
And a patch on `app.GUIManager::isNewBenefit()` to always return false. (This should make the many exclamation point checks the DLC cat (Support Desk) does much faster. Basically no DLC will appear as a new exclamation point.)

Here's a native plugin (put in `reframework\plugins\`).<br>
This will just do a very simple cache of the function result so it doesn't have to call over and over.<br>
This will work with your current DLC without granting you all the DLC in the world.<br>

PS: If it isn't obvs. to you, using this means you have to restart the game on purchasing DLC for the new stuff to be found.