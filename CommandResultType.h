#ifndef COMMANDRESULTTYPE_H
#define COMMANDRESULTTYPE_H

enum CommandResultType : quint8 {
    CT_CHECK_FILES_NO_FILES,
    CT_CHECK_FILES_MISMATCH,
    CT_CHECK_FILES_SUCCESS,
    CT_PREMIUM_LOGIN_SUCCESS,
    CT_PREMIUM_LOGIN_FAIL,
    CT_CONNECTION_ERROR,
    CT_CLIENT_DOWNLOADING_ERROR,
    CT_CLIENT_DOWNLOADING_SUCCESS,
    CT_CLIENT_UNZIP_ERROR,
    CT_CLIENT_UNZIP_SUCCESS,
    CT_CLIENT_FILE_WRITING_ERROR,
    CT_CLIENT_GET_REMOTE_DATA_ERROR,
    CT_CLIENT_GET_REMOTE_DATA_SUCCESS,
    CT_LOCAL_SETTINGS_LOADING_FAIL,
    CT_LOCAL_SETTINGS_LOADING_SUCCESS,
    CT_LOCAL_SETTINGS_SAVING_FAIL,
    CT_LOCAL_SETTINGS_SAVING_SUCCESS,
    CT_CLIENT_DIR_CHANGE_SUCCESS
};

#endif // COMMANDRESULTTYPE_H
