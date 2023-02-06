package org.gpxsee.gpxsee;

import android.content.Intent;

public class Activity extends org.qtproject.qt.android.bindings.QtActivity
{
	@Override
	public void onNewIntent(Intent intent)
	{
		setIntent(intent);
	}

	public String intentPath()
	{
		String path = "";

		Intent intent = getIntent();
		if (intent != null) {
			if (intent.getAction() == Intent.ACTION_VIEW)
				path = intent.getDataString();
			setIntent(null);
		}

		return path;
	}
}
