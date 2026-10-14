package org.gpxsee.gpxsee;

import android.content.ContentResolver;
import android.content.Context;
import android.database.Cursor;
import android.net.Uri;
import android.provider.DocumentsContract;
import java.util.ArrayList;

public class FileUtils {

	public static String[] listDir(Context context, String treeUriString) {
		if (context == null || treeUriString == null || treeUriString.isEmpty())
			return new String[0];

		try {
			Uri treeUri = Uri.parse(treeUriString);
			String treeDocId = DocumentsContract.getTreeDocumentId(treeUri);
			Uri childrenUri = DocumentsContract.buildChildDocumentsUriUsingTree(treeUri, treeDocId);
			ContentResolver resolver = context.getContentResolver();
			ArrayList<String> result = new ArrayList<>();
			String[] projection = new String[] {
				DocumentsContract.Document.COLUMN_DOCUMENT_ID,
				DocumentsContract.Document.COLUMN_MIME_TYPE
			};

			Cursor cursor = resolver.query(childrenUri, projection, null, null, null);
			if (cursor == null)
				return new String[0];

			int idIndex = cursor.getColumnIndexOrThrow(DocumentsContract.Document.COLUMN_DOCUMENT_ID);
			int mimeIndex = cursor.getColumnIndexOrThrow(DocumentsContract.Document.COLUMN_MIME_TYPE);

			while (cursor.moveToNext()) {
				String childId = cursor.getString(idIndex);
				String mimeType = cursor.getString(mimeIndex);

				if (!DocumentsContract.Document.MIME_TYPE_DIR.equals(mimeType)) {
					Uri childUri = DocumentsContract.buildDocumentUriUsingTree(treeUri, childId);
					result.add(childUri.toString());
				}
			}

			return result.toArray(new String[0]);
		} catch (Exception e) {
			return new String[0];
		}
	}
}
