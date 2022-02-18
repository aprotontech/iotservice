'use strict';

export default class Commons {

    static handleApiError(el, error) {
        el.$message.error({
            showClose: true,
            message: error.toString()
        });
        console.error(error);
    }


}

